#include <node.h>
#include <v8.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <nan.h>

using namespace v8;

#define AVZ_DEFINE_CONSTANT(target, name, value) \
		(target)->Set(Nan::New(name).ToLocalChecked(), Nan::New(value))

#define AVZ_FILL_TIMESPEC(target, sec, nsec) \
		(target)->Set(Nan::New("sec").ToLocalChecked(), Nan::New<Number>(sec)); \
		(target)->Set(Nan::New("nsec").ToLocalChecked(), Nan::New<Number>(nsec));

#define AVZ_VALIDATE_ARG_CLOCKID(arg) \
		if(!(arg)->IsInt32()) { \
			Nan::ThrowTypeError("Specified clockId is not supported on this system"); \
			return; \
		}

NAN_METHOD(ClockGetTime) {
	if(info.Length() != 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	AVZ_VALIDATE_ARG_CLOCKID(info[0]);

	clockid_t clockId = (clockid_t)info[0]->Int32Value();
	struct timespec ts;

	if(clock_gettime(clockId, &ts) != 0) {
		if(errno == EINVAL) {
			Nan::ThrowTypeError("Specified clockId is not supported on this system");
		} else {
			Nan::ThrowError(Nan::ErrnoException(errno, "clock_gettime", ""));
		}

		return;
	}

	Local<Object> obj = Nan::New<Object>();

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	info.GetReturnValue().Set(obj);
}

NAN_METHOD(ClockGetRes) {
	if(info.Length() != 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}

	AVZ_VALIDATE_ARG_CLOCKID(info[0]);

	clockid_t clockId = (clockid_t)info[0]->Int32Value();
	struct timespec ts;

	if(clock_getres(clockId, &ts) != 0) {
		if(errno == EINVAL) {
			Nan::ThrowTypeError("Specified clockId is not supported on this system");
		} else {
			Nan::ThrowError(Nan::ErrnoException(errno, "clock_getres", ""));
		}

		return;
	}

	Local<Object> obj = Nan::New<Object>();

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	info.GetReturnValue().Set(obj);
}

NAN_METHOD(ClockNanosleep) {
	if(info.Length() != 3) {
		Nan::ThrowTypeError("Wrong number of arguments");

		return;
	}

	AVZ_VALIDATE_ARG_CLOCKID(info[0]);

	clockid_t clockId = (clockid_t)info[0]->Int32Value();

	if(!info[1]->IsInt32()) {
		Nan::ThrowTypeError("Specified flags is not supported on this system");
		return;
	}

	int flags = info[1]->Int32Value();

	if(!info[2]->IsObject()) {
		Nan::ThrowTypeError("Sleep time must be an object, e.g. {sec: 1212, nsec: 4344}");

		return;
	}

	struct timespec sleepTimeTs;
	struct timespec remainingTimeTs;

	Local<Object> objSleep = info[2]->ToObject();
	Local<Value> secValue = objSleep->Get(Nan::New("sec").ToLocalChecked());
	Local<Value> nsecValue = objSleep->Get(Nan::New("nsec").ToLocalChecked());

	if(!secValue->IsUndefined() && !secValue->IsUint32()) {
		Nan::ThrowTypeError("Option `sec` must be unsigned integer");

		return;
	}

	if(!nsecValue->IsUndefined() && !nsecValue->IsUint32()) {
		Nan::ThrowTypeError("Option `nsec` must be unsigned integer");

		return;
	}

	sleepTimeTs.tv_sec = (time_t)secValue->Uint32Value();
	sleepTimeTs.tv_nsec = (long)nsecValue->Uint32Value();

	if(sleepTimeTs.tv_nsec < 0 || sleepTimeTs.tv_nsec >= 1e9) {
		Nan::ThrowTypeError("Option `nsec` must be in [0; 999999999]");

		return;
	}

#ifdef __linux__
	int err = clock_nanosleep(clockId, flags, &sleepTimeTs, &remainingTimeTs);

	if(err != 0) {
		if(err == EINVAL) {
			Nan::ThrowTypeError("Specified clockId is not supported on this system or invalid argument");
		} else if(err == EINTR) {
			/* stopped by signal - need to return remaining time */
			struct timespec *res;

			if(flags & TIMER_ABSTIME) {
				res = &sleepTimeTs;
			} else {
				res = &remainingTimeTs;
			}

			Local<Object> obj = Nan::New<Object>();

			AVZ_FILL_TIMESPEC(obj, res->tv_sec, res->tv_nsec);

			info.GetReturnValue().Set(obj);
		} else {
			Nan::ThrowError(Nan::ErrnoException(errno, "clock_nanosleep", ""));

			return;
		}
	}
#else
	if(clockId != CLOCK_REALTIME) {
		Nan::ThrowTypeError("Only nanosleep(REALTIME) clock is supported by your OS");

		return;
	}

	if(flags) {
		Nan::ThrowTypeError("Specified flags is not supported on this system");
		return;
	}

	int err = nanosleep(&sleepTimeTs, &remainingTimeTs);

	if(err == -1) {
		if(errno == EINTR) {
			Local<Object> obj = Nan::New<Object>();

			AVZ_FILL_TIMESPEC(obj, remainingTimeTs.tv_sec, remainingTimeTs.tv_nsec);

			info.GetReturnValue().Set(obj);
		} else {
			Nan::ThrowError(Nan::ErrnoException(errno, "nanosleep", ""));

			return;
		}
	}
#endif

	return;
}

extern "C"
void init(Handle<Object> exports) {
	Nan::SetMethod(exports, "gettime", ClockGetTime);
	Nan::SetMethod(exports, "getres", ClockGetRes);
	Nan::SetMethod(exports, "nanosleep", ClockNanosleep);

#ifdef TIMER_ABSTIME
	AVZ_DEFINE_CONSTANT(exports, "TIMER_ABSTIME", TIMER_ABSTIME); // for nanosleep
#endif

	AVZ_DEFINE_CONSTANT(exports, "REALTIME", CLOCK_REALTIME);
	AVZ_DEFINE_CONSTANT(exports, "MONOTONIC", CLOCK_MONOTONIC);

	/* Linux-specific constants */
#ifdef CLOCK_REALTIME_COARSE
	AVZ_DEFINE_CONSTANT(exports, "REALTIME_COARSE", CLOCK_REALTIME_COARSE);
#endif

#ifdef CLOCK_MONOTONIC_COARSE
	AVZ_DEFINE_CONSTANT(exports, "MONOTONIC_COARSE", CLOCK_MONOTONIC_COARSE);
#endif

#ifdef CLOCK_MONOTONIC_RAW
	AVZ_DEFINE_CONSTANT(exports, "MONOTONIC_RAW", CLOCK_MONOTONIC_RAW);
#endif

#ifdef CLOCK_BOOTTIME
	AVZ_DEFINE_CONSTANT(exports, "BOOTTIME", CLOCK_BOOTTIME);
#endif

#ifdef CLOCK_PROCESS_CPUTIME_ID
	AVZ_DEFINE_CONSTANT(exports, "PROCESS_CPUTIME_ID", CLOCK_PROCESS_CPUTIME_ID);
#endif

#ifdef CLOCK_THREAD_CPUTIME_ID
	AVZ_DEFINE_CONSTANT(exports, "THREAD_CPUTIME_ID", CLOCK_THREAD_CPUTIME_ID);
#endif

	/* FreeBSD-specific constants */
#ifdef CLOCK_REALTIME_FAST
	AVZ_DEFINE_CONSTANT(exports, "REALTIME_FAST", CLOCK_REALTIME_FAST);
#endif

#ifdef CLOCK_REALTIME_PRECISE
	AVZ_DEFINE_CONSTANT(exports, "REALTIME_PRECISE", CLOCK_REALTIME_PRECISE);
#endif

#ifdef CLOCK_MONOTONIC_FAST
	AVZ_DEFINE_CONSTANT(exports, "MONOTONIC_FAST", CLOCK_MONOTONIC_FAST);
#endif

#ifdef CLOCK_MONOTONIC_PRECISE
	AVZ_DEFINE_CONSTANT(exports, "MONOTONIC_PRECISE", CLOCK_MONOTONIC_PRECISE);
#endif

#ifdef CLOCK_UPTIME
	AVZ_DEFINE_CONSTANT(exports, "UPTIME", CLOCK_UPTIME);
#endif

#ifdef CLOCK_UPTIME_FAST
	AVZ_DEFINE_CONSTANT(exports, "UPTIME_FAST", CLOCK_UPTIME_FAST);
#endif

#ifdef CLOCK_UPTIME_PRECISE
	AVZ_DEFINE_CONSTANT(exports, "THREAD_UPTIME_PRECISE", CLOCK_UPTIME_PRECISE);
#endif

#ifdef CLOCK_SECOND
	AVZ_DEFINE_CONSTANT(exports, "THREAD_SECOND", CLOCK_SECOND);
#endif

#ifdef CLOCK_PROF
	AVZ_DEFINE_CONSTANT(exports, "PROF", CLOCK_PROF);
#endif
}

//--

NODE_MODULE(posix_clock, init)
