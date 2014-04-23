#include "zopfli.h"

#include <node_buffer.h>
#include <node_version.h>

#include <string.h>  // memcpy

#include <string>

#include "./zopfli-binding.h"

namespace nodezopfli {


using namespace v8;
using namespace node;


Handle<Value> parseOptions(const Handle<Object>& options, ZopfliOptions& zopfli_options) {

  Local<Value> fieldValue;

  //NanScope();

  Handle<Value> error = NanNull();

  // Whether to print output
  fieldValue = options->Get(NanNew<v8::String>("verbose"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.verbose = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      error = Exception::TypeError(NanNew<v8::String>("Wrong type for option 'verbose'"));
    }
  }

  // Whether to print more detailed output
  fieldValue = options->Get(NanNew<v8::String>("verbose_more"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.verbose_more = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      error = Exception::TypeError(NanNew<v8::String>("Wrong type for option 'verbose_more'"));
    }
  }

  /*
  Maximum amount of times to rerun forward and backward pass to optimize LZ77
  compression cost. Good values: 10, 15 for small files, 5 for files over
  several MB in size or it will be too slow.
  */
  fieldValue = options->Get(NanNew<v8::String>("numiterations"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsInt32() && fieldValue->ToInt32()->Value() > 0) {
      zopfli_options.numiterations = fieldValue->ToInt32()->Value();
    } else {
      //Wrong
      error = Exception::TypeError(NanNew<v8::String>("Wrong type for option 'numiterations'"));
    }
  }

  /*
  If true, splits the data in multiple deflate blocks with optimal choice
  for the block boundaries. Block splitting gives better compression. Default:
  true (1).
  */
  fieldValue = options->Get(NanNew<v8::String>("blocksplitting"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.blocksplitting = (fieldValue->ToBoolean()->Value()) ? 1 : 0;
    } else {
      //Wrong
      error = Exception::TypeError(NanNew<v8::String>("Wrong type for option 'blocksplitting'"));
    }
  }

  /*
  If true, chooses the optimal block split points only after doing the iterative
  LZ77 compression. If false, chooses the block split points first, then does
  iterative LZ77 on each individual block. Depending on the file, either first
  or last gives the best compression. Default: false (0).
  */
  fieldValue = options->Get(NanNew<v8::String>("blocksplittinglast"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.blocksplittinglast = (fieldValue->ToBoolean()->Value()) ? 1 : 0;
    } else {
      //Wrong
      error = Exception::TypeError(NanNew<v8::String>("Wrong type for option 'blocksplittinglast'"));
    }
  }

  /*
  Maximum amount of blocks to split into (0 for unlimited, but this can give
  extreme results that hurt compression on some files). Default value: 15.
  */
  fieldValue = options->Get(NanNew<v8::String>("blocksplittingmax"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsInt32()) {
      zopfli_options.blocksplittingmax = fieldValue->ToInt32()->Value();
    } else {
      error = Exception::TypeError(NanNew<v8::String>("Wrong type for option 'blocksplittingmax'"));
    }
  }
  return (error);
}

  Handle<Value> ParseArgs(_NAN_METHOD_ARGS_TYPE args, ZopfliFormat& format, ZopfliOptions& zopfli_options) {
    Handle<Value> error = NanNull();

    if(args.Length() < 1 || !Buffer::HasInstance(args[0])) {
      return Exception::TypeError(NanNew<v8::String>("First argument must be a buffer"));
    }

    format = ZOPFLI_FORMAT_DEFLATE;
    if(args.Length() >= 2 && args[1]->IsString()) {
      std::string given_format(*String::AsciiValue(args[1]->ToString()));
      if(given_format.compare("gzip") == 0) {
        format = ZOPFLI_FORMAT_GZIP;
      } else if(given_format.compare("zlib") == 0) {
        format = ZOPFLI_FORMAT_ZLIB;
      } else if(given_format.compare("deflate") == 0) {
        format = ZOPFLI_FORMAT_DEFLATE;
      } else {
        return Exception::TypeError(NanNew<v8::String>("Invalid Zopfli format"));
      }
    } else {
      return Exception::TypeError(NanNew<v8::String>("Second argument must be a string"));
    }

    if(error->IsNull()) {
      if(args.Length() >= 3 && args[2]->IsObject()) {
        Handle<Object> options = Handle<Object>::Cast(args[2]);
        return NanNew<v8::Value>(parseOptions(options, zopfli_options));
      } else {
        return Exception::TypeError(NanNew<v8::String>("Third argument must be an object"));
      }
    }
    return error;
  }

  template<class T> ZopfliRequest<T>::ZopfliRequest(_NAN_METHOD_ARGS) {
    NanScope();
    ZopfliInitOptions(&zopfli_options);
    v8::Handle<v8::Object> object = args[0]->ToObject();
    size_t length = node::Buffer::Length(object);
    const char *data = node::Buffer::Data(object);
    input = std::string(data, length);
    resultdata = 0;
    resultsize = 0;
    callback = new NanCallback(v8::Local<v8::Function>::Cast(args[args.Length()-1]));
    err = NULL;
  }

//const std::string ZopfliErrors::kInvalidInput = "Invalid input";

// Base
// PROTECTED
inline void Base::CallCallback(const v8::Handle<v8::Function>& callback,
                                  const v8::Handle<v8::Value>& err,
                                  const v8::Handle<v8::Value>& res) {
  v8::Handle<v8::Value> argv[2] = {err, res};
  callback->Call(v8::Context::GetCurrent()->Global(), 2, argv);
}

inline void Base::CallErrCallback(const v8::Handle<v8::Function>& callback,
                                  const v8::Handle<v8::Value>& err) {
  //v8::Handle<v8::Value> err =
  //  v8::Exception::Error(v8::String::New(str.data(), str.length()));
  v8::Handle<v8::Value> res = NanNew<v8::Value>(NanNull());
  CallCallback(callback, err, res);
}

void CompressBinding::After(uv_work_t *req) {
  NanScope();

  ZopfliRequest<std::string>* zopfli_req = static_cast<ZopfliRequest<std::string>*>(req->data);
  if (zopfli_req->err != NULL) {
    v8::Handle<v8::Value> err =
      v8::Exception::Error(NanNew<String>(zopfli_req->err->data(), zopfli_req->err->length()));
    CallErrCallback(zopfli_req->callback->GetFunction(), err);
  } else {
    CallOkCallback(zopfli_req->callback->GetFunction(), zopfli_req->resultdata, zopfli_req->resultsize);
  }

  uv_unref((uv_handle_t*) req);
  delete zopfli_req->callback;
  delete zopfli_req;
  delete req;
}

inline void
CompressBinding::CallOkCallback(const v8::Handle<v8::Function>& callback,
                                const char* data, size_t size) {
  v8::Handle<v8::Value> err = NanNew<v8::Value>(NanNull());
  v8::Local<v8::Object> res = NanNewBufferHandle((char*)data, size);
  CallCallback(callback, err, res);
}

// CompressBinding
// PUBLIC
NAN_METHOD(CompressBinding::Async) {
  NanScope();
  ZopfliRequest<std::string>* zopfli_req = new ZopfliRequest<std::string>(args);

  if(args.Length() == 0 || (args.Length() >= 1 && !args[args.Length()-1]->IsFunction())) {
    return NanThrowError(Exception::TypeError(NanNew<v8::String>("Last argument must be a callback function")));
  }

  Local<Value> error = NanNew<v8::Value>(ParseArgs(args, zopfli_req->format, zopfli_req->zopfli_options));
  if (!error->IsNull()) {
    CallErrCallback(v8::Local<v8::Function>::Cast(args[args.Length()-1]), error);
    NanReturnUndefined();
  }

  uv_work_t* _req = new uv_work_t;
  _req->data = zopfli_req;
  uv_queue_work(uv_default_loop(), _req, AsyncOperation, (uv_after_work_cb)After);
  NanReturnUndefined();
}

NAN_METHOD(CompressBinding::Sync) {
  NanScope();
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  ZopfliInitOptions(&zopfli_options);
  Local<Value> error = NanNew<v8::Value>(ParseArgs(args, format, zopfli_options));
  if(!error->IsNull()) {
    ThrowException(error);
    NanReturnUndefined();
  }
  Local<Value> inbuffer = args[0];
  size_t inbuffersize = Buffer::Length(inbuffer->ToObject());
  const unsigned char * inbufferdata = (const unsigned char*)Buffer::Data(inbuffer->ToObject());
  unsigned char* out = 0;
  size_t outsize = 0;
  ZopfliCompress(&zopfli_options, format,
    inbufferdata, inbuffersize,
    &out, &outsize);

  Local<Object> actualBuffer = NanNewBufferHandle((char*)out, outsize);
  NanReturnValue(actualBuffer);
}

// PRIVATE
void CompressBinding::AsyncOperation(uv_work_t *req) {
  ZopfliRequest<std::string>* zopfli_req = static_cast<ZopfliRequest<std::string>*>(req->data);
  std::string* input = &zopfli_req->input;
  ZopfliCompress(&zopfli_req->zopfli_options, zopfli_req->format,
    (const unsigned char*)input->data(), input->length(), (unsigned char **)&zopfli_req->resultdata, &zopfli_req->resultsize);
}


unsigned updateAdler32(unsigned int adler, const unsigned char* data, size_t size)
{
  unsigned sums_overflow = 5550;
  unsigned s1 = adler;
  unsigned s2 = 1 >> 16;

  while (size > 0) {
    size_t amount = size > sums_overflow ? sums_overflow : size;
    size -= amount;
    while (amount > 0) {
      s1 += (*data++);
      s2 += s1;
      amount--;
    }
    s1 %= 65521;
    s2 %= 65521;
  }
  return (s2 << 16) | s1;
}

NAN_METHOD(Adler32) {
  NanScope();

  if(args.Length() >= 1 && !args[0]->IsUint32() && !args[0]->IsInt32()) {
    return NanThrowError(Exception::TypeError(NanNew<v8::String>("adler must be an unsigned integer")));
  }

  unsigned int adler = args[0]->Uint32Value();

  if(args.Length() < 1 || !Buffer::HasInstance(args[1])) {
    return NanThrowError(Exception::TypeError(NanNew<String>("data must be a buffer")));
  }
  Local<Value> inbuffer = args[1];
  size_t inbuffersize = Buffer::Length(inbuffer->ToObject());
  const unsigned char * data = (const unsigned char*)Buffer::Data(inbuffer->ToObject());
  adler = updateAdler32(adler, data, inbuffersize);
  NanReturnValue(Integer::NewFromUnsigned(adler));
}


extern "C" void
init(v8::Handle<v8::Object> exports) {
  NODE_SET_METHOD(exports, "deflate", CompressBinding::Async);
  NODE_SET_METHOD(exports, "deflateSync", CompressBinding::Sync);
  NODE_SET_METHOD(exports, "adler32", Adler32);
}

NODE_MODULE(zopfli, init)
}
