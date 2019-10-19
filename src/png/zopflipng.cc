#include <napi.h>

#include "lodepng/lodepng.h"
#include "zopflipng_lib.h"

using namespace Napi;

void parseOptions(const Napi::Object& options, ZopfliPNGOptions& png_options) {
  Napi::Value option_value;
  const Napi::Env env = options.Env();

  if(!options.IsEmpty()) {
    // Allow altering hidden colors of fully transparent pixels
    if (options.Has("lossy_transparent")) {
      option_value = options.Get("lossy_transparent");
      if(!option_value.IsNumber()) {
        Napi::TypeError::New(env, "Wrong type for option 'lossy_transparent'").ThrowAsJavaScriptException();

      }
      png_options.lossy_transparent = option_value.As<Napi::Number>().Int32Value();
    }

    // Convert 16-bit per channel images to 8-bit per channel
    if (options.Has("lossy_8bit")) {
      option_value = options.Get("lossy_8bit");
      if(!option_value.IsNumber()) {
        Napi::TypeError::New(env, "Wrong type for option 'lossy_8bit'").ThrowAsJavaScriptException();

      }
      png_options.lossy_8bit = option_value.As<Napi::Number>().Int32Value();
    }

    // Filter strategies to try
    //"zero", "one", "two", "three", "four", "minimum", "entropy", "predefined", "brute"
    Napi::Value fieldValue = options.Get("filter_strategies");
    if(!fieldValue.IsUndefined() && !fieldValue.IsNull()) {
      if(fieldValue.IsArray()) {
        Array filter_strategies = fieldValue.As<Array>();
        for (uint32_t i = 0; i < filter_strategies.Length(); i++) {
          std::string strStrategy(filter_strategies.Get(i).ToString());
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
            Napi::TypeError::New(options.Env(), (std::string("Wrong strategy : ") + strStrategy).c_str()).ThrowAsJavaScriptException();
          }
          png_options.filter_strategies.push_back(strategy);
        }
      } else {
        //Wrong
        Napi::TypeError::New(env, "Wrong type for option 'filter_strategies'").ThrowAsJavaScriptException();
      }
    }

    // Automatically choose filter strategy using less good compression
    if (options.Has("auto_filter_strategy")) {
      option_value = options.Get("auto_filter_strategy");
      if(!option_value.IsNumber()) {
        Napi::TypeError::New(env, "Wrong type for option 'auto_filter_strategy'").ThrowAsJavaScriptException();
      }
      png_options.auto_filter_strategy = option_value.As<Napi::Number>().Int32Value();
    }

    // PNG chunks to keep
    // chunks to literally copy over from the original PNG to the resulting one
    if (options.Has("use_zopfli")) {
      option_value = options.Get("use_zopfli");
      if(!option_value.IsNumber()) {
        Napi::TypeError::New(env, "Wrong type for option 'use_zopfli'").ThrowAsJavaScriptException();
      }
      png_options.use_zopfli = option_value.As<Napi::Number>().Int32Value();
    }

    // Zopfli number of iterations
    if (options.Has("num_iterations")) {
      option_value = options.Get("num_iterations");
      if(!option_value.IsNumber()) {
        Napi::TypeError::New(env, "Wrong type for option 'num_iterations'").ThrowAsJavaScriptException();
      }
      png_options.num_iterations = option_value.As<Napi::Number>().Int32Value();
    }

    // Zopfli number of iterations on images > 200 KiB
    if (options.Has("num_iterations_large")) {
      option_value = options.Get("num_iterations_large");
      if(!option_value.IsNumber()) {
        Napi::TypeError::New(env, "Wrong type for option 'num_iterations_large'").ThrowAsJavaScriptException();
      }
      png_options.num_iterations_large = option_value.As<Napi::Number>().Int32Value();
    }

    // Split chunk strategy none, first, last, both
    std::string strStrategy;
    if (options.Has("block_split_strategy")) {
      option_value = options.Get("block_split_strategy");
      if(!option_value.IsString()) {
        Napi::TypeError::New(env, "Wrong type for option 'block_split_strategy'").ThrowAsJavaScriptException();
      }
      if(strStrategy.compare("none") == 0) { png_options.block_split_strategy = 0; }
      else if(strStrategy.compare("first") == 0) { png_options.block_split_strategy = 1; }
      else if(strStrategy.compare("last") == 0) { png_options.block_split_strategy = 2; }
      else if(strStrategy.compare("both") == 0) { png_options.block_split_strategy = 3; }
      else {
        Napi::TypeError::New(env, "Wrong value for option 'block_split_strategy'").ThrowAsJavaScriptException();
      }
    }
  }
}


Napi::Value PNGDeflate(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  if(info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "First argument must be a string").ThrowAsJavaScriptException();
  }
  std::string imageName(info[0].As<Napi::String>().Utf8Value().c_str());

  if(info.Length() < 2 || !info[1].IsString()) {
    Napi::TypeError::New(env, "First argument must be a string").ThrowAsJavaScriptException();
  }
  std::string out_filename(info[1].As<Napi::String>().Utf8Value().c_str());

  ZopfliPNGOptions png_options;

  if(info.Length() >= 2 && info[2].IsObject()) {
    Napi::Object options = info[2].ToObject();
    parseOptions(options, png_options);
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
    printf("Decoding error %u: %s\n", error, lodepng_error_text(error));
  } else {
    // Verify result, check that the result causes no decoding errors
    error = lodepng::decode(image, w, h, inputstate, resultpng);
    if (error) {
      printf("Error: verification of result failed.\n");
    } else {
      lodepng::save_file(resultpng, out_filename);
    }
  }
  return Napi::Number::New(env, error);
}