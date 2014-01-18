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
		ThrowException(Exception::Error(String::New("Invalid argument")));
		return scope.Close(Undefined());
	}

	clockid_t clockId = args[0]->Int32Value();
	struct timespec ts;

	if(clock_gettime(clockId, &ts) != 0) {
		ThrowException(Exception::Error(String::New(strerror(errno))));
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
		ThrowException(Exception::Error(String::New("Invalid argument")));
		return scope.Close(Undefined());
	}

	clockid_t clockId = args[0]->Int32Value();
	struct timespec ts;

	if(clock_getres(clockId, &ts) != 0) {
		ThrowException(Exception::Error(String::New(strerror(errno))));
		return scope.Close(Undefined());
	}

	Local<Object> obj = Object::New();

	AVZ_FILL_TIMESPEC(obj, ts.tv_sec, ts.tv_nsec);

	return scope.Close(obj);
}

void init(Handle<Object> exports) {
	exports->Set(String::NewSymbol("gettime"), FunctionTemplate::New(ClockGetTime)->GetFunction());
	exports->Set(String::NewSymbol("getres"), FunctionTemplate::New(ClockGetRes)->GetFunction());

	AVZ_DEFINE_CONSTANT(exports, "REALTIME", CLOCK_REALTIME);
	AVZ_DEFINE_CONSTANT(exports, "MONOTONIC", CLOCK_MONOTONIC);

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
}

//--

NODE_MODULE(posix_clock, init)
