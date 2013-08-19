#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include <iostream>
#include <cstring>

#include "./png/zopflipng.h"
#include "zopfli.h"

using namespace v8;
using namespace node;

Handle<Value> parseOptions(const Handle<Object>& options, ZopfliOptions& zopfli_options) {
  
  Local<Value> fieldValue;

  HandleScope handle_scope;
  Handle<Value> error = Null();

  // Whether to print output
  fieldValue = options->Get(String::New("verbose"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.verbose = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      error = Exception::TypeError(String::New("Wrong type for option 'verbose'"));
    }
  }

  // Whether to print more detailed output
  fieldValue = options->Get(String::New("verbose_more"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.verbose_more = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      error = Exception::TypeError(String::New("Wrong type for option 'verbose_more'"));
    }
  }

  /*
  Maximum amount of times to rerun forward and backward pass to optimize LZ77
  compression cost. Good values: 10, 15 for small files, 5 for files over
  several MB in size or it will be too slow.
  */
  fieldValue = options->Get(String::New("numiterations"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsInt32()) {
      zopfli_options.numiterations = fieldValue->ToInt32()->Value();
    } else {
      //Wrong
      error = Exception::TypeError(String::New("Wrong type for option 'numiterations'"));
    }
  }

  /*
  If true, splits the data in multiple deflate blocks with optimal choice
  for the block boundaries. Block splitting gives better compression. Default:
  true (1).
  */
  fieldValue = options->Get(String::New("blocksplitting"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.blocksplitting = (fieldValue->ToBoolean()->Value()) ? 1 : 0;
    } else {
      //Wrong
      error = Exception::TypeError(String::New("Wrong type for option 'blocksplitting'"));
    }
  }

  /*
  If true, chooses the optimal block split points only after doing the iterative
  LZ77 compression. If false, chooses the block split points first, then does
  iterative LZ77 on each individual block. Depending on the file, either first
  or last gives the best compression. Default: false (0).
  */
  fieldValue = options->Get(String::New("blocksplittinglast"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      zopfli_options.blocksplittinglast = (fieldValue->ToBoolean()->Value()) ? 1 : 0;
    } else {
      //Wrong
      error = Exception::TypeError(String::New("Wrong type for option 'blocksplittinglast'"));
    }
  }

  /*
  Maximum amount of blocks to split into (0 for unlimited, but this can give
  extreme results that hurt compression on some files). Default value: 15.
  */
  fieldValue = options->Get(String::New("blocksplittingmax"));
  if(error->IsNull() && !fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsInt32()) {
      zopfli_options.blocksplittingmax = fieldValue->ToInt32()->Value();
    } else {
      error = Exception::TypeError(String::New("Wrong type for option 'blocksplittingmax'"));
    }
  }
  return handle_scope.Close(error);
}

Handle<Value> ParseArgs(const Arguments& args, ZopfliFormat& format, ZopfliOptions& zopfli_options) {
  HandleScope handle_scope;  
  Handle<Value> error = Null();
  if(args.Length() < 1 || !Buffer::HasInstance(args[0])) {
    error = Exception::TypeError(String::New("First argument must be a buffer"));
  }
  
  format = ZOPFLI_FORMAT_DEFLATE;
  if(error->IsNull()) {
    if(args.Length() >= 2 && args[1]->IsString()) {
      std::string given_format(*String::AsciiValue(args[1]->ToString()));
      if(given_format.compare("gzip") == 0) {
        format = ZOPFLI_FORMAT_GZIP;
      } else if(given_format.compare("zlib") == 0) {
        format = ZOPFLI_FORMAT_ZLIB;
      } else if(given_format.compare("deflate") == 0) {
        format = ZOPFLI_FORMAT_DEFLATE;
      } else {
        error = Exception::TypeError(String::New("Invalid format"));
      }
    } else {
      error = Exception::TypeError(String::New("Second argument must be a string"));
    }
  }

  if(error->IsNull()) {
    if(args.Length() >= 3 && args[2]->IsObject()) {
      Handle<Object> options = Handle<Object>::Cast(args[2]);
      error = Local<Value>::New(parseOptions(options, zopfli_options));
    } else {
      error = Exception::TypeError(String::New("Third argument must be an object"));
    }
  }
  return handle_scope.Close(error);
}


Handle<Value> DeflateSync(const Arguments& args) {
  HandleScope scope;
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  ZopfliInitOptions(&zopfli_options);
  Local<Value> error = Local<Value>::New(ParseArgs(args, format, zopfli_options));
  if(!error->IsNull()) {
    ThrowException(error);
    return scope.Close(Undefined());
  }
  Local<Value> inbuffer = args[0];
  size_t inbuffersize = Buffer::Length(inbuffer->ToObject());
  const unsigned char * inbufferdata = (const unsigned char*)Buffer::Data(inbuffer->ToObject());
  unsigned char* out = 0;
  size_t outsize = 0;
  ZopfliCompress(&zopfli_options, format, 
    inbufferdata, inbuffersize,
    &out, &outsize);

  Buffer *slowBuffer = Buffer::New(outsize);
  memcpy(Buffer::Data(slowBuffer), (char*)out, outsize);
  Local<Object> globalObj = Context::GetCurrent()->Global();
  Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
  Handle<Value> constructorArgs[3] = { slowBuffer->handle_, Integer::New(outsize), Integer::New(0) };
  Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
  return scope.Close(actualBuffer);
} 

Handle<Value> Deflate(const Arguments& args) {
  HandleScope scope;
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  ZopfliInitOptions(&zopfli_options);
  Local<Value> error = Local<Value>::New(ParseArgs(args, format, zopfli_options));
  if(error->IsNull()) {
    //Callback function
    if(args.Length() >= 1 && !args[args.Length()-1]->IsFunction()) {
      return ThrowException(Exception::TypeError(String::New("Last argument must be a callback function")));
    }
    Local<Function> callback = Local<Function>::Cast(args[args.Length()-1]);
    Local<Value> inbuffer = args[0];
    size_t inbuffersize = Buffer::Length(inbuffer->ToObject());
    const unsigned char * inbufferdata = (const unsigned char*)Buffer::Data(inbuffer->ToObject());
    unsigned char* out = 0;
    size_t outsize = 0;
    ZopfliCompress(&zopfli_options, format, 
      inbufferdata, inbuffersize,
      &out, &outsize);

    Local<Buffer> buf = Buffer::New((char*)out, outsize);
    Local<Value> argv[] = {
        Local<Value>::New(Null()),
        Local<Value>::New(buf->handle_)
    };
    callback->Call(Context::GetCurrent()->Global(), 2, argv);
    return scope.Close(Undefined());
  } else {
    return scope.Close(error);
  }
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

Handle<Value> Adler32(const Arguments& args) {
  HandleScope scope;

  if(args.Length() >= 1 && !args[0]->IsUint32() && !args[0]->IsInt32()) {
    return ThrowException(Exception::TypeError(String::New("adler must be an unsigned integer")));
  }

  unsigned int adler = args[0]->Uint32Value();
  
  if(args.Length() < 1 || !Buffer::HasInstance(args[1])) {
    return ThrowException(Exception::TypeError(String::New("data must be a buffer")));
  }
  Local<Value> inbuffer = args[1];
  size_t inbuffersize = Buffer::Length(inbuffer->ToObject());
  const unsigned char * data = (const unsigned char*)Buffer::Data(inbuffer->ToObject());
  adler = updateAdler32(adler, data, inbuffersize);
  return scope.Close(Integer::NewFromUnsigned(adler));
}

void init(Handle<Object> target) {
  target->Set(String::NewSymbol("pngcompress"),
      FunctionTemplate::New(PNGDeflate)->GetFunction());
  target->Set(String::NewSymbol("deflate"),
      FunctionTemplate::New(Deflate)->GetFunction());
  target->Set(String::NewSymbol("deflateSync"),
      FunctionTemplate::New(DeflateSync)->GetFunction());
  target->Set(String::NewSymbol("adler32"),
      FunctionTemplate::New(Adler32)->GetFunction());
}
NODE_MODULE(zopfli, init)
