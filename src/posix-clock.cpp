#include "napi.h"
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#define AVZ_DEFINE_CONSTANT(env, target, name, value) (target).Set(Napi::String::New(env, name), Napi::Number::New(env, (unsigned long)value))

#define AVZ_FILL_TIMESPEC(target, sec, nsec) \
		(target).Set("sec", sec); \
		(target).Set("nsec", nsec);

#define AVZ_THROW(env, message) Napi::TypeError::New(env, message).ThrowAsJavaScriptException()

#define AVZ_THROW_ERRNO(env, message) AVZ_THROW(env, strerror(errno))

#define AVZ_VALIDATE_ARG_CLOCKID(env, arg) \
		if(!(arg).IsNumber()) { \
			AVZ_THROW(env, "Specified clockId is not supported on this system"); \
			return env.Undefined(); \
		}

Napi::Value ClockGetTime(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if(info.Length() != 1) {
		AVZ_THROW(env, "Wrong number of arguments");

		return env.Undefined();
	}

	AVZ_VALIDATE_ARG_CLOCKID(info.Env(), info[0]);

	clockid_t clockId = (clockid_t)info[0].ToNumber().Int32Value();
	struct timespec ts;

	if(clock_gettime(clockId, &ts) != 0) {
		if(errno == EINVAL) {
			AVZ_THROW(env, "Specified clockId is not supported on this system");
		} else {
			AVZ_THROW_ERRNO(env, "clock_gettime");
		}

		return env.Undefined();
	}

	Napi::Object obj = Napi::Object::New(env);

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	return obj;
}

Napi::Value ClockGetRes(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if(info.Length() != 1) {
		AVZ_THROW(env, "Wrong number of arguments");
		return env.Undefined();
	}

	AVZ_VALIDATE_ARG_CLOCKID(env, info[0]);

	clockid_t clockId = (clockid_t)info[0].ToNumber().Int32Value();
	struct timespec ts;

	if(clock_getres(clockId, &ts) != 0) {
		if(errno == EINVAL) {
			AVZ_THROW(env, "Specified clockId is not supported on this system");
		} else {
			AVZ_THROW_ERRNO(env, "clock_getres");
		}

		return env.Undefined();
	}

	Napi::Object obj = Napi::Object::New(env);

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	return obj;
}

Napi::Value ClockNanosleep(const Napi::CallbackInfo& info)
{
	Napi::Env env = info.Env();

	if(info.Length() != 3) {
		AVZ_THROW(env, "Wrong number of arguments");

		return env.Undefined();
	}

	AVZ_VALIDATE_ARG_CLOCKID(env, info[0]);

	clockid_t clockId = (clockid_t)info[0].ToNumber().Int32Value();

	if(!info[1].IsNumber()) {
		AVZ_THROW(env, "Specified flags is not supported on this system");
		return env.Undefined();
	}

	int flags = info[1].ToNumber().Int32Value();

	if(!info[2].IsObject()) {
		AVZ_THROW(env, "Sleep time must be an object, e.g. {sec: 1212, nsec: 4344}");

		return env.Undefined();
	}

	struct timespec sleepTimeTs;
	struct timespec remainingTimeTs;

	Napi::Object objSleep = info[2].ToObject();
	Napi::Value secValue = objSleep.Get("sec");
	Napi::Value nsecValue = objSleep.Get("nsec");

	if(!secValue.IsUndefined() && !secValue.IsNumber()) {
		AVZ_THROW(env, "Option `sec` must be unsigned integer");

		return env.Undefined();
	}

	if(!nsecValue.IsUndefined() && !nsecValue.IsNumber()) {
		AVZ_THROW(env, "Option `nsec` must be unsigned integer");

		return env.Undefined();
	}

	sleepTimeTs.tv_sec = (time_t)secValue.ToNumber().Uint32Value();
	sleepTimeTs.tv_nsec = (long)nsecValue.ToNumber().Uint32Value();

	if(sleepTimeTs.tv_nsec < 0 || sleepTimeTs.tv_nsec >= 1e9) {
		AVZ_THROW(env, "Option `nsec` must be in [0; 999999999]");

		return env.Undefined();
	}

#ifdef __linux__
	int err = clock_nanosleep(clockId, flags, &sleepTimeTs, &remainingTimeTs);

	if(err != 0) {
		if(err == EINVAL) {
			AVZ_THROW(env, "Specified clockId is not supported on this system or invalid argument");
		} else if(err == EINTR) {
			/* stopped by signal - need to return remaining time */
			struct timespec *res;

			if(flags & TIMER_ABSTIME) {
				res = &sleepTimeTs;
			} else {
				res = &remainingTimeTs;
			}

			Napi::Object obj = Napi::Object::New(env);

			AVZ_FILL_TIMESPEC(obj, res->tv_sec, res->tv_nsec);

			return obj;
		} else {
			AVZ_THROW(env, "clock_nanosleep");

			return env.Undefined();
		}
	}
#else
	if(clockId != CLOCK_REALTIME) {
		AVZ_THROW(env, "Only nanosleep(REALTIME) clock is supported by your OS");

		return env.Undefined();
	}

	if(flags) {
		AVZ_THROW(env, "Specified flags is not supported on this system");
		return env.Undefined();
	}

	int err = nanosleep(&sleepTimeTs, &remainingTimeTs);

	if(err == -1) {
		if(errno == EINTR) {
			Napi::Object obj = Napi::Object::New(env);

			AVZ_FILL_TIMESPEC(obj, remainingTimeTs.tv_sec, remainingTimeTs.tv_nsec);

			return obj;
		} else {
			AVZ_THROW_ERRNO(env, "nanosleep");

			return env.Undefined();
		}
	}
#endif

	return env.Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
	exports.Set(Napi::String::New(env, "gettime"), Napi::Function::New(env, ClockGetTime));
	exports.Set(Napi::String::New(env, "getres"), Napi::Function::New(env, ClockGetRes));
	exports.Set(Napi::String::New(env, "nanosleep"), Napi::Function::New(env, ClockNanosleep));

#ifdef TIMER_ABSTIME
	AVZ_DEFINE_CONSTANT(env, exports, "TIMER_ABSTIME", TIMER_ABSTIME); // for nanosleep
#endif

	AVZ_DEFINE_CONSTANT(env, exports, "REALTIME", CLOCK_REALTIME);
	AVZ_DEFINE_CONSTANT(env, exports, "MONOTONIC", CLOCK_MONOTONIC);

	/* Linux-specific constants */
#ifdef CLOCK_REALTIME_COARSE
	AVZ_DEFINE_CONSTANT(env, exports, "REALTIME_COARSE", CLOCK_REALTIME_COARSE);
#endif

#ifdef CLOCK_MONOTONIC_COARSE
	AVZ_DEFINE_CONSTANT(env, exports, "MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE);
#endif

#ifdef CLOCK_MONOTONIC_RAW
	AVZ_DEFINE_CONSTANT(env, exports, "MONOTONIC_RAW", CLOCK_MONOTONIC_RAW);
#endif

#ifdef CLOCK_BOOTTIME
	AVZ_DEFINE_CONSTANT(env, exports, "BOOTTIME", CLOCK_BOOTTIME);
#endif

#ifdef CLOCK_PROCESS_CPUTIME_ID
	AVZ_DEFINE_CONSTANT(env, exports, "PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
#endif

#ifdef CLOCK_THREAD_CPUTIME_ID
	AVZ_DEFINE_CONSTANT(env, exports, "THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
#endif

	/* FreeBSD-specific constants */
#ifdef CLOCK_REALTIME_FAST
	AVZ_DEFINE_CONSTANT(env, exports, "REALTIME_FAST", CLOCK_REALTIME_FAST);
#endif

#ifdef CLOCK_REALTIME_PRECISE
	AVZ_DEFINE_CONSTANT(env, exports, "REALTIME_PRECISE", CLOCK_REALTIME_PRECISE);
#endif

#ifdef CLOCK_MONOTONIC_FAST
	AVZ_DEFINE_CONSTANT(env, exports, "MONOTONIC_FAST", CLOCK_MONOTONIC_FAST);
#endif

#ifdef CLOCK_MONOTONIC_PRECISE
	AVZ_DEFINE_CONSTANT(env, exports, "MONOTONIC_PRECISE", CLOCK_MONOTONIC_PRECISE);
#endif

#ifdef CLOCK_UPTIME
	AVZ_DEFINE_CONSTANT(env, exports, "UPTIME", CLOCK_UPTIME);
#endif

#ifdef CLOCK_UPTIME_FAST
	AVZ_DEFINE_CONSTANT(env, exports, "UPTIME_FAST", CLOCK_UPTIME_FAST);
#endif

#ifdef CLOCK_UPTIME_PRECISE
	AVZ_DEFINE_CONSTANT(env, exports, "THREAD_UPTIME_PRECISE", CLOCK_UPTIME_PRECISE);
#endif

#ifdef CLOCK_SECOND
	AVZ_DEFINE_CONSTANT(env, exports, "THREAD_SECOND", CLOCK_SECOND);
#endif

#ifdef CLOCK_PROF
	AVZ_DEFINE_CONSTANT(env, exports, "PROF", CLOCK_PROF);
#endif

	return exports;
}

//--

NODE_API_MODULE(posix_clock, Init)
