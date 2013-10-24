// Copyright 2013 Mark Cavage <mcavage@gmail.com> All rights reserved.

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include <node.h>
#include <v8.h>

using namespace v8;



///--- V8 Nonsense

#define RETURN_EXCEPTION(MSG)                                           \
	return v8::ThrowException(v8::Exception::Error(v8::String::New(MSG)))

#define RETURN_ARGS_EXCEPTION(MSG)                                      \
	return v8::ThrowException(v8::Exception::TypeError(v8::String::New(MSG)))

#define REQUIRE_ARGS(ARGS)			      \
	if (ARGS.Length() == 0)					\
		RETURN_ARGS_EXCEPTION("missing arguments");

#define REQUIRE_STRING_ARG(ARGS, I, VAR)			      \
	REQUIRE_ARGS(ARGS);                                           \
	if (ARGS.Length() <= (I) || !ARGS[I]->IsString())		\
		RETURN_ARGS_EXCEPTION("argument " #I " must be a String"); \
	v8::String::Utf8Value VAR(ARGS[I]->ToString());

#define REQUIRE_INT_ARG(ARGS, I, VAR)			      \
	REQUIRE_ARGS(ARGS);                                           \
	if (ARGS.Length() <= (I) || !ARGS[I]->IsInt32())		\
		RETURN_ARGS_EXCEPTION("argument " #I " must be an Integer"); \
	v8::Local<v8::Int32> VAR = ARGS[I]->ToInt32();

#define REQUIRE_FUNCTION_ARG(ARGS, I, VAR)                              \
	REQUIRE_ARGS(ARGS);						\
	if (ARGS.Length() <= (I) || !ARGS[I]->IsFunction())                   \
		RETURN_EXCEPTION("argument " #I " must be a Function");	\
	v8::Local<v8::Function> VAR = v8::Local<v8::Function>::Cast(ARGS[I]);



///--- libuv "baton"

class my_baton_t {
public:
	my_baton_t() {
		_errno = 0;
		name = 0;
		path = NULL;
		value = 0;
	};

	virtual ~my_baton_t() {
		if (path != NULL)
			free(path);
		path = NULL;
	};

	// Inputs
	char *path;
	Persistent<Function> callback;

	// Output
	int _errno;
	int name;
	long value;

private:
	my_baton_t(const my_baton_t &);
	my_baton_t &operator=(const my_baton_t &);
};



///--- Async APIs

void _pathconf (uv_work_t *req) {
	my_baton_t *baton = (my_baton_t *)req->data;

	baton->value = pathconf(baton->path, baton->name);
	if (baton->value != 0)
		baton->_errno = errno;
}


void _pathconf_after (uv_work_t *req, int ignore_me_status_in_0_dot_10) {
	HandleScope scope;


	int argc = 1;
	Handle<Value> argv[2];

	my_baton_t *baton = (my_baton_t *)req->data;
	if (baton->value < 0) {
		argv[0] = node::ErrnoException(baton->_errno, "pathconf");
	} else {
		argc = 2;
		argv[0] = Local<Value>::New(Null());
		argv[1] = Number::New(baton->value);
	}

	TryCatch try_catch;
	baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);
	if (try_catch.HasCaught())
		node::FatalException(try_catch);

	baton->callback.Dispose();
	delete baton;
	delete req;
}


Handle<Value> _pathconf_entry(const Arguments& args) {
	HandleScope scope;

	REQUIRE_STRING_ARG(args, 0, path);
	REQUIRE_INT_ARG(args, 1, name);
	REQUIRE_FUNCTION_ARG(args, 2, callback);

	my_baton_t *baton = new my_baton_t;
	if (!baton)
		RETURN_EXCEPTION("out of memory");

	baton->callback = Persistent<Function>::New(callback);
	baton->path = strdup(*path);
	baton->name = name->Value();
	if (!baton->path) {
		delete baton;
		RETURN_EXCEPTION("out of memory");
	}

	uv_work_t *req = new uv_work_t;
	if (req == NULL) {
		delete baton;
		RETURN_EXCEPTION("out of memory");
	}

	req->data = baton;
	uv_queue_work(uv_default_loop(), req, _pathconf,
	    (uv_after_work_cb)_pathconf_after);

	return Undefined();
}



///--- Actually expose this to the outside world

extern "C" {
	static void init(Handle<Object> target) {
		HandleScope scope;

		NODE_DEFINE_CONSTANT(target, _PC_LINK_MAX);
		NODE_DEFINE_CONSTANT(target, _PC_MAX_INPUT);
		NODE_DEFINE_CONSTANT(target, _PC_NAME_MAX);
		NODE_DEFINE_CONSTANT(target, _PC_PATH_MAX);
		NODE_DEFINE_CONSTANT(target, _PC_PIPE_BUF);
		NODE_DEFINE_CONSTANT(target, _PC_CHOWN_RESTRICTED);
		NODE_DEFINE_CONSTANT(target, _PC_NO_TRUNC);
		NODE_DEFINE_CONSTANT(target, _PC_VDISABLE);
		NODE_DEFINE_CONSTANT(target, _PC_XATTR_SIZE_BITS);

		NODE_SET_METHOD(target, "pathconf", _pathconf_entry);
	}

	NODE_MODULE(pathconf, init)
}
