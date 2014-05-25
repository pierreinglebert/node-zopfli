#include <node.h>
#include <v8.h>

#include <iostream>
#include "nan.h"

#include "lodepng/lodepng.h"
#include "zopflipng_lib.h"

using namespace v8;
using namespace node;

NAN_INLINE bool GetOptionIfType(
    v8::Local<v8::Object> optionsObj
  , v8::Handle<v8::String> opt
  , bool* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    Local<Value> optval = optionsObj->Get(opt);
    if(optval->IsBoolean()) {
      *def = optval->BooleanValue();
    } else {
      return false;
    }
  }
  return true;
}

NAN_INLINE bool GetOptionIfType(
    v8::Local<v8::Object> optionsObj
  , v8::Handle<v8::String> opt
  , uint32_t* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    Local<Value> optval = optionsObj->Get(opt);
    if(optval->IsNumber()) {
      *def = optval->Uint32Value();
    } else {
      return false;
    }
  }
  return true;
}

NAN_INLINE bool GetOptionIfType(
    v8::Local<v8::Object> optionsObj
  , v8::Handle<v8::String> opt
  , std::string* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    Local<Value> optval = optionsObj->Get(opt);
    if(optval->IsString()) {
      *def = std::string(NanCString(optval, NULL));
    } else {
      return false;
    }
  }
  return true;
}

NAN_INLINE bool GetOptionIfType(
    v8::Local<v8::Object> optionsObj
  , v8::Handle<v8::String> opt
  , int32_t* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    Local<Value> optval = optionsObj->Get(opt);
    if(optval->IsNumber()) {
      *def = optval->Int32Value();
    } else {
      return false;
    }
  }
  return true;
}

#define _THROW(type, errmsg) \
  NanThrowError(_NAN_ERROR(type, errmsg));

using namespace v8;

bool parseOptions(const Local<Object>& options, ZopfliPNGOptions& png_options) {
  Handle<Value> fieldValue;

  // Allow altering hidden colors of fully transparent pixels
  if(!GetOptionIfType(options, NanNew<String>("lossy_transparent"), &png_options.lossy_transparent)) {
    _THROW(Exception::TypeError, "Wrong type for option 'lossy_transparent'");
    return false;
  }

  // Convert 16-bit per channel images to 8-bit per channel
  if(!GetOptionIfType(options, NanNew<String>("lossy_8bit"), &png_options.lossy_8bit)) {
    _THROW(Exception::TypeError, "Wrong type for option 'lossy_8bit'");
    return false;
  }

  // Filter strategies to try
  //"zero", "one", "two", "three", "four", "minimum", "entropy", "predefined", "brute"
  fieldValue = options->Get(NanNew<String>("filter_strategies"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsArray()) {
      Handle<Array> filter_strategies = Handle<Array>::Cast(fieldValue);
      for (uint32_t i = 0; i < filter_strategies->Length(); i++) {
        size_t count;
        std::string strStrategy(NanCString(filter_strategies->Get(i)->ToString(), &count));
        ZopfliPNGFilterStrategy strategy = kStrategyZero;
        if(strStrategy.compare("zero") == 0) { strategy = kStrategyZero; }
        else if(strStrategy.compare("one") == 0) { strategy = kStrategyOne; }
        else if(strStrategy.compare("two") == 0) { strategy = kStrategyTwo; }
        else if(strStrategy.compare("three") == 0) { strategy = kStrategyThree; }
        else if(strStrategy.compare("four") == 0) { strategy = kStrategyFour; }
        else if(strStrategy.compare("minsum") == 0) { strategy = kStrategyMinSum; }
        else if(strStrategy.compare("entropy") == 0) { strategy = kStrategyEntropy; }
        else if(strStrategy.compare("predefined") == 0) { strategy = kStrategyPredefined; }
        else if(strStrategy.compare("bruteforce") == 0) { strategy = kStrategyBruteForce; }
        else {
          _THROW(Exception::TypeError, (std::string("Wrong strategy : ") + strStrategy).c_str());
          return false;
        }
        png_options.filter_strategies.push_back(strategy);
      }
    } else {
      //Wrong
      _THROW(Exception::TypeError, "Wrong type for option 'filter_strategies'");
      return false;
    }
  }

  // Automatically choose filter strategy using less good compression
  if(!GetOptionIfType(options, NanNew<String>("auto_filter_strategy"), &png_options.auto_filter_strategy)) {
    _THROW(Exception::TypeError, "Wrong type for option 'auto_filter_strategy'");
    return false;
  }

  // PNG chunks to keep
  // chunks to literally copy over from the original PNG to the resulting one
  if(!GetOptionIfType(options, NanNew<String>("use_zopfli"), &png_options.use_zopfli)) {
    _THROW(Exception::TypeError, "Wrong type for option 'use_zopfli'");
    return false;
  }

  // Zopfli number of iterations
  if(!GetOptionIfType(options, NanNew<String>("num_iterations"), &png_options.num_iterations)) {
    _THROW(Exception::TypeError, "Wrong type for option 'num_iterations'");
    return false;
  }

  // Zopfli number of iterations on images > 200ko
  if(!GetOptionIfType(options, NanNew<String>("num_iterations_large"), &png_options.num_iterations_large)) {
    _THROW(Exception::TypeError, "Wrong type for option 'num_iterations_large'");
    return false;
  }

  // Split chunk strategy none, first, last, both
  std::string strStrategy;
  if(GetOptionIfType(options, NanNew<String>("block_split_strategy"), &strStrategy)) {
    if(strStrategy.compare("none") == 0) { png_options.block_split_strategy = 0; }
    else if(strStrategy.compare("first") == 0) { png_options.block_split_strategy = 1; }
    else if(strStrategy.compare("last") == 0) { png_options.block_split_strategy = 2; }
    else if(strStrategy.compare("both") == 0) { png_options.block_split_strategy = 3; }
    else {
      _THROW(Exception::TypeError, "Wrong value for option 'block_split_strategy'");
      return false;
    }
  } else {
    _THROW(Exception::TypeError, "Wrong type for option 'block_split_strategy'");
    return false;
  }

  return true;
}


NAN_METHOD(PNGDeflate) {
  NanScope();

  if(args.Length() < 1 || !args[0]->IsString()) {
    _THROW(Exception::TypeError, "First argument must be a string");
    NanReturnUndefined();
  }
  size_t count;
  std::string imageName(NanCString(args[0]->ToString(), &count));

  if(args.Length() < 2 || !args[1]->IsString()) {
    _THROW(Exception::TypeError, "First argument must be a string");
    NanReturnUndefined();
  }
  std::string out_filename(NanCString(args[1]->ToString(), &count));

  ZopfliPNGOptions png_options;

  if(args.Length() >= 2 && args[2]->IsObject()) {
    Local<Object> options = args[2]->ToObject();
    if(!parseOptions(options, png_options)) {
      NanReturnUndefined();
    }
  }

  std::vector<unsigned char> image;
  unsigned w, h;
  std::vector<unsigned char> origpng;
  unsigned error;
  lodepng::State inputstate;
  std::vector<unsigned char> resultpng;

  lodepng::load_file(origpng, imageName);

  bool verbose = false;
  error = ZopfliPNGOptimize(origpng, png_options, verbose, &resultpng);

  if (error) {
    printf("Decoding error %i: %s\n", error, lodepng_error_text(error));
  } else {
    // Verify result, check that the result causes no decoding errors
    error = lodepng::decode(image, w, h, inputstate, resultpng);
    if (error) {
      printf("Error: verification of result failed.\n");
    } else {
      lodepng::save_file(resultpng, out_filename);
    }
  }
  NanReturnValue(NanNew<Integer>(error));
}
