#include "zopfli.h"

#include <string.h>  // memcpy
#include <string>

#include "./zopfli-binding.h"

namespace nodezopfli {

using namespace v8;
using namespace node;

NAN_INLINE _NAN_METHOD_RETURN_TYPE ParseArgs(_NAN_METHOD_ARGS, ZopfliFormat& format, ZopfliOptions& zopfli_options) {
  ZopfliInitOptions(&zopfli_options);

  if(args.Length() < 1 || !Buffer::HasInstance(args[0])) {
    _THROW(Exception::TypeError, "First argument must be a buffer");
  }

  format = ZOPFLI_FORMAT_DEFLATE;
  if(args.Length() >= 2 && args[1]->IsString()) {
    size_t count;
    std::string given_format(NanCString(args[1]->ToString(), &count));
    if(given_format.compare("gzip") == 0) {
      format = ZOPFLI_FORMAT_GZIP;
    } else if(given_format.compare("zlib") == 0) {
      format = ZOPFLI_FORMAT_ZLIB;
    } else if(given_format.compare("deflate") == 0) {
      format = ZOPFLI_FORMAT_DEFLATE;
    } else {
      _THROW(Exception::TypeError, "Invalid Zopfli format");
    }
  } else {
    _THROW(Exception::TypeError, "Second argument must be a string");
  }

  if(args.Length() >= 3 && args[2]->IsObject()) {
    Local<Object> options = Local<Object>::Cast(args[2]);
    Local<Value> fieldValue;

    // Whether to print output
    GetOptionIfExists(options, NanNew<String>("verbose"), &zopfli_options.verbose);
    // Whether to print more detailed output
    GetOptionIfExists(options, NanNew<String>("verbose_more"), &zopfli_options.verbose_more);
    /*
    Maximum amount of times to rerun forward and backward pass to optimize LZ77
    compression cost. Good values: 10, 15 for small files, 5 for files over
    several MB in size or it will be too slow.
    */
    GetOptionIfExists(options, NanNew<String>("numiterations"), &zopfli_options.numiterations);
    /*
    If true, chooses the optimal block split points only after doing the iterative
    LZ77 compression. If false, chooses the block split points first, then does
    iterative LZ77 on each individual block. Depending on the file, either first
    or last gives the best compression. Default: false (0).
    */
    GetOptionIfExists(options, NanNew<String>("blocksplitting"), &zopfli_options.blocksplitting);
    /*
    If true, chooses the optimal block split points only after doing the iterative
    LZ77 compression. If false, chooses the block split points first, then does
    iterative LZ77 on each individual block. Depending on the file, either first
    or last gives the best compression. Default: false (0).
    */
    GetOptionIfExists(options, NanNew<String>("blocksplittinglast"), &zopfli_options.blocksplittinglast);
    /*
    Maximum amount of blocks to split into (0 for unlimited, but this can give
    extreme results that hurt compression on some files). Default value: 15.
    */
    GetOptionIfExists(options, NanNew<String>("blocksplittingmax"), &zopfli_options.blocksplittingmax);
  } else {
    _THROW(Exception::TypeError, "Third argument must be an object");
  }
  NanReturnUndefined();
}


// Base
// PROTECTED
class CompressWorker : public NanAsyncWorker {
 public:
  CompressWorker(NanCallback *callback, ZopfliFormat& format, ZopfliOptions& zopfli_options, Handle<Object> buffer)
  : NanAsyncWorker(callback), format(format), zopfli_options(zopfli_options) {
    NanScope();
    // Handle<Object> object = args[0]->ToObject();
    size_t length = node::Buffer::Length(buffer);
    const char *data = node::Buffer::Data(buffer);
    input = std::string(data, length);
    resultdata = 0;
    resultsize = 0;
  }
  ~CompressWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute() {
    std::string* input = &this->input;
    ZopfliCompress(&zopfli_options, format, (const unsigned char*)input->data(), input->length(), (unsigned char **)&resultdata, &resultsize);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback() {
    NanScope();

    Local<Value> argv[] = {
      NanNew(NanNull()),
      NanNewBufferHandle((char*)resultdata, resultsize)
    };

    callback->Call(2, argv);
  }

 private:
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  std::string input;
  char* resultdata;
  size_t resultsize;
};

// CompressBinding
// PUBLIC
NAN_METHOD(CompressBinding::Async) {
  NanScope();
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  if(args.Length() == 0 || (args.Length() >= 1 && !args[args.Length()-1]->IsFunction())) {
    return _THROW(Exception::TypeError, "Last argument must be a callback function");
  }
  ParseArgs(args, format, zopfli_options);

  NanCallback *callback = new NanCallback(args[args.Length()-1].As<v8::Function>());
  NanAsyncQueueWorker(new CompressWorker(callback, format, zopfli_options, args[0]->ToObject()));
  NanReturnUndefined();
}

NAN_METHOD(CompressBinding::Sync) {
  NanScope();
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  ParseArgs(args, format, zopfli_options);
  Local<Object> inbuffer = args[0]->ToObject();
  size_t inbuffersize = Buffer::Length(inbuffer);
  const unsigned char * inbufferdata = (const unsigned char*)Buffer::Data(inbuffer);
  unsigned char* out = 0;
  size_t outsize = 0;
  ZopfliCompress(&zopfli_options, format,
    inbufferdata, inbuffersize,
    &out, &outsize);

  Local<Object> actualBuffer = NanNewBufferHandle((char*)out, outsize);
  NanReturnValue(actualBuffer);
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
    return NanThrowError(_NAN_ERROR(Exception::TypeError, "adler must be an unsigned integer"));
  }

  unsigned int adler = args[0]->Uint32Value();

  if(args.Length() < 1 || !Buffer::HasInstance(args[1])) {
    return NanThrowError(_NAN_ERROR(Exception::TypeError, "data must be a buffer"));
  }
  Local<Value> inbuffer = args[1];
  size_t inbuffersize = Buffer::Length(inbuffer->ToObject());
  const unsigned char * data = (const unsigned char*)Buffer::Data(inbuffer->ToObject());
  adler = updateAdler32(adler, data, inbuffersize);
  NanReturnValue(NanNew<Uint32>(adler));
}


extern "C" void
init(v8::Handle<v8::Object> exports) {
  NODE_SET_METHOD(exports, "deflate", CompressBinding::Async);
  NODE_SET_METHOD(exports, "deflateSync", CompressBinding::Sync);
  NODE_SET_METHOD(exports, "adler32", Adler32);
}

NODE_MODULE(zopfli, init)
}
