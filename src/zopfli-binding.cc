#include <napi.h>
#include "zopfli.h"
#include "zopfli-binding.h"
#include "png/zopflipng.h"

namespace nodezopfli {

using namespace Napi;

inline void ParseArgs(const Napi::CallbackInfo& info, ZopfliFormat& format, ZopfliOptions& zopfli_options) {
  ZopfliInitOptions(&zopfli_options);
  format = ZOPFLI_FORMAT_GZIP;
  const Napi::Env env = info.Env();

  if(info.Length() < 1 || !info[0].IsBuffer()) {
    Napi::TypeError::New(env, "First argument must be a buffer").ThrowAsJavaScriptException();
  }

  if(info.Length() >= 2 && info[1].IsString()) {
    std::string given_format(info[1].As<Napi::String>().Utf8Value().c_str());
    if(given_format.compare("gzip") == 0) {
      format = ZOPFLI_FORMAT_GZIP;
    } else if(given_format.compare("zlib") == 0) {
      format = ZOPFLI_FORMAT_ZLIB;
    } else if(given_format.compare("deflate") == 0) {
      format = ZOPFLI_FORMAT_DEFLATE;
    } else {
      Napi::TypeError::New(env, "Invalid Zopfli format").ThrowAsJavaScriptException();
    }
  } else {
    Napi::TypeError::New(env, "Second argument must be a string").ThrowAsJavaScriptException();
  }

  if(info.Length() >= 3 && info[2].IsObject()) {
     Napi::Object options = info[2].As<Napi::Object>();

    if (!options.IsEmpty()) {
      Napi::Value fieldValue;

      // Whether to print output
      if (options.Has("verbose")) {
        zopfli_options.verbose = options.Get("verbose").As<Napi::Boolean>().Value();
      }

      // Whether to print more detailed output
      if (options.Has("verbose_more")) {
        zopfli_options.verbose_more = options.Get("verbose_more").As<Napi::Boolean>().Value();
      }

      /*
      Maximum amount of times to rerun forward and backward pass to optimize LZ77
      compression cost. Good values: 10, 15 for small files, 5 for files over
      several MB in size or it will be too slow.
      */
      if (options.Has("numiterations")) {
        zopfli_options.numiterations = options.Get("numiterations").As<Napi::Number>().Int32Value();
      }

      /*
      If true, chooses the optimal block split points only after doing the iterative
      LZ77 compression. If false, chooses the block split points first, then does
      iterative LZ77 on each individual block. Depending on the file, either first
      or last gives the best compression. Default: false (0).
      */
      if (options.Has("blocksplitting")) {
        zopfli_options.blocksplitting = options.Get("blocksplitting").As<Napi::Boolean>().Value();
      }

      /*
      If true, chooses the optimal block split points only after doing the iterative
      LZ77 compression. If false, chooses the block split points first, then does
      iterative LZ77 on each individual block. Depending on the file, either first
      or last gives the best compression. Default: false (0).
      */
      if (options.Has("blocksplittinglast")) {
        zopfli_options.blocksplittinglast = options.Get("blocksplittinglast").As<Napi::Boolean>().Value();
      }

      /*
      Maximum amount of blocks to split into (0 for unlimited, but this can give
      extreme results that hurt compression on some files). Default value: 15.
      */
      if (options.Has("blocksplittingmax")) {
        zopfli_options.blocksplittingmax = options.Get("blocksplittingmax").As<Napi::Number>().Uint32Value();
      }
    }
  } else {
    Napi::TypeError::New(env, "Third argument must be an object").ThrowAsJavaScriptException();
  }
}

// Base
// PROTECTED
class CompressWorker : public Napi::AsyncWorker {
 public:
  CompressWorker(const Napi::Function callback, ZopfliFormat& format, ZopfliOptions& zopfli_options, Napi::Buffer<unsigned char> buffer)
  : Napi::AsyncWorker(callback), format(format), zopfli_options(zopfli_options) {
    Napi::HandleScope scope(Env());
    input = std::string((char*) buffer.Data(), buffer.Length());
    resultdata = 0;
    resultsize = 0;
  }
  ~CompressWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute() override {
    std::string* input = &this->input;
    ZopfliCompress(&zopfli_options, format, (const unsigned char*)input->data(), input->length(), (unsigned char **)&resultdata, &resultsize);
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void OnOK() override {
    Napi::HandleScope scope(Env());
    Callback().MakeCallback(
      Receiver().Value(),
      {
        Env().Null(),
        Napi::Buffer<unsigned char>::New(Env(), (unsigned char*)resultdata, resultsize)
      });
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
Napi::Value CompressBinding::Async(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  if(info.Length() == 0 || (info.Length() >= 1 && !info[info.Length()-1].IsFunction())) {
    Napi::TypeError::New(env, "Last argument must be a callback function").ThrowAsJavaScriptException();
  }
  ParseArgs(info, format, zopfli_options);
  Napi::Function callback = info[info.Length() - 1].As<Napi::Function>();
  CompressWorker* worker = new CompressWorker(callback, format, zopfli_options, info[0].As<Napi::Buffer<unsigned char>>());
  worker->Queue();
  return env.Undefined();
}

Napi::Value CompressBinding::Sync(const Napi::CallbackInfo& info) {
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  ParseArgs(info, format, zopfli_options);
  Napi::Buffer<unsigned char> inbuffer = info[0].As<Napi::Buffer<unsigned char>>();
  size_t inbuffersize = inbuffer.Length();
  const unsigned char * inbufferdata = inbuffer.Data();
  unsigned char* out = 0;
  size_t outsize = 0;
  ZopfliCompress(&zopfli_options, format,
    inbufferdata, inbuffersize,
    &out, &outsize);
  return Napi::Buffer<unsigned char>::New(info.Env(), out, outsize);
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

Napi::Value Adler32(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  if(info.Length() >= 1 && !info[0].IsNumber()) {
    Napi::TypeError::New(env, "adler must be an unsigned integer").ThrowAsJavaScriptException();
    return env.Null();
  }
  unsigned int adler = info[0].As<Napi::Number>().Uint32Value();
  if(info.Length() < 1 || !info[1].IsBuffer()) {
    Napi::TypeError::New(env, "data must be a buffer").ThrowAsJavaScriptException();
    return env.Null();
  }
  Napi::Buffer<unsigned char> inbuffer = info[1].As<Napi::Buffer<unsigned char>>();
  size_t inbuffersize = inbuffer.Length();
  const unsigned char * data = inbuffer.Data();
  adler = updateAdler32(adler, data, inbuffersize);
  return Napi::Number::New(env, adler);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "deflate"), Napi::Function::New(env, CompressBinding::Async));
  exports.Set(Napi::String::New(env, "deflateSync"), Napi::Function::New(env, CompressBinding::Sync));
  exports.Set(Napi::String::New(env, "adler32"), Napi::Function::New(env, Adler32));
  exports.Set(Napi::String::New(env, "pngcompress"), Napi::Function::New(env, PNGDeflate));
  return exports;
}

NODE_API_MODULE(zopfli, Init)

}
