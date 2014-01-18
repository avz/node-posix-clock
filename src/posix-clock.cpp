#include <node.h>
#include <v8.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

using namespace v8;

#define AVZ_DEFINE_CONSTANT(target, name, value) \
		(target)->Set(v8::String::NewSymbol(name), \
		v8::Integer::New(value), \
		static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))

#define AVZ_FILL_TIMESPEC(target, sec, nsec) \
		(target)->Set(String::NewSymbol("sec"), Number::New(sec)); \
		(target)->Set(String::NewSymbol("nsec"), Integer::NewFromUnsigned(static_cast<uint32_t>(nsec)));

Handle<Value> ClockGetTime(const Arguments& args) {
	HandleScope scope;

	if(args.Length() != 1) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}

	if(!args[0]->IsInt32()) {
		ThrowException(Exception::Error(String::New("Specified clockId is not supported on this system")));
		return scope.Close(Undefined());
	}

	clockid_t clockId = args[0]->Int32Value();
	struct timespec ts;

	if(clock_gettime(clockId, &ts) != 0) {
		if(errno == EINVAL)
			ThrowException(Exception::Error(String::New("Specified clockId is not supported on this system")));
		else
			ThrowException(Exception::Error(String::Concat(String::New(strerror(errno)), args[0]->ToString())));

		return scope.Close(Undefined());
	}

	Local<Object> obj = Object::New();

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	return scope.Close(obj);
}

Handle<Value> ClockGetRes(const Arguments& args) {
	HandleScope scope;

	if(args.Length() != 1) {
		ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
		return scope.Close(Undefined());
	}

	if(!args[0]->IsInt32()) {
		ThrowException(Exception::Error(String::New("Specified clockId is not supported on this system")));
		return scope.Close(Undefined());
	}

	clockid_t clockId = args[0]->Int32Value();
	struct timespec ts;

	if(clock_getres(clockId, &ts) != 0) {
		if(errno == EINVAL)
			ThrowException(Exception::Error(String::New("Specified clockId is not supported on this system")));
		else
			ThrowException(Exception::Error(String::Concat(String::New(strerror(errno)), args[0]->ToString())));

		return scope.Close(Undefined());
	}

	Local<Object> obj = Object::New();

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	return scope.Close(obj);
}

extern "C"
void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("gettime"), FunctionTemplate::New(ClockGetTime)->GetFunction());
	exports->Set(String::NewSymbol("getres"), FunctionTemplate::New(ClockGetRes)->GetFunction());

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
