#include <node.h>
#include <v8.h>

#include <iostream>


#include "lodepng/lodepng.h"
#include "zopflipng_lib.h"

using namespace v8;

bool parseOptions(const Handle<Object>& options, ZopfliPNGOptions& png_options) {
  
  Handle<Value> fieldValue;

  // Allow altering hidden colors of fully transparent pixels
  fieldValue = options->Get(String::New("lossy_transparent"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      png_options.lossy_transparent = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'lossy_transparent'")));
      return false;
    }
  }

  // Convert 16-bit per channel images to 8-bit per channel
  fieldValue = options->Get(String::New("lossy_8bit"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      png_options.lossy_8bit = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'lossy_8bit'")));
      return false;
    }
  }

  // Filter strategies to try
  //"zero", "one", "two", "three", "four", "minimum", "entropy", "predefined", "brute"
  fieldValue = options->Get(String::New("filter_strategies"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsArray()) {
      Handle<Array> filter_strategies = Handle<Array>::Cast(fieldValue);
      for (uint32_t i = 0; i < filter_strategies->Length(); i++) {
        std::string strStrategy(*String::AsciiValue(filter_strategies->Get(Integer::New(i))->ToString()));
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
          ThrowException(Exception::TypeError(String::Concat(String::New("Wrong strategy : "), String::New(strStrategy.c_str()))));
          return false;
        }
        png_options.filter_strategies.push_back(strategy);
      }
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'filter_strategies'")));
      return false;
    }
  }

  
  // Automatically choose filter strategy using less good compression
  fieldValue = options->Get(String::New("auto_filter_strategy"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      png_options.auto_filter_strategy = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'auto_filter_strategy'")));
      return false;
    }
  }

  // PNG chunks to keep
  // chunks to literally copy over from the original PNG to the resulting one
  fieldValue = options->Get(String::New("keepchunks"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsArray()) {
      Handle<Array> keepchunks = Handle<Array>::Cast(fieldValue);
      for (uint32_t i = 0; i < keepchunks->Length(); i++) {
        String::AsciiValue s(keepchunks->Get(Integer::New(i))->ToString());
        png_options.keepchunks.push_back(std::string(*s));
      }
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'keepchunks'")));
      return false;
    }
  }

  // Use Zopfli deflate compression
  fieldValue = options->Get(String::New("use_zopfli"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsBoolean()) {
      png_options.use_zopfli = fieldValue->ToBoolean()->Value();
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'use_zopfli'")));
      return false;
    }
  }

  // Zopfli number of iterations
  fieldValue = options->Get(String::New("num_iterations"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsInt32()) {
      png_options.num_iterations = fieldValue->ToInt32()->Value();
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'num_iterations'")));
      return false;
    }
  }

  // Zopfli number of iterations on images > 200ko
  fieldValue = options->Get(String::New("num_iterations_large"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsInt32()) {
      png_options.num_iterations_large = fieldValue->ToInt32()->Value();
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'num_iterations_large'")));
      return false;
    }
  }

  // Split chunk strategy none, first, last, both
  fieldValue = options->Get(String::New("block_split_strategy"));
  if(!fieldValue->IsUndefined() && !fieldValue->IsNull()) {
    if(fieldValue->IsString()) {
      std::string strStrategy(*String::AsciiValue(fieldValue->ToString()));
      if(strStrategy.compare("none") == 0) { png_options.block_split_strategy = 0; }
      else if(strStrategy.compare("first") == 0) { png_options.block_split_strategy = 1; }
      else if(strStrategy.compare("last") == 0) { png_options.block_split_strategy = 2; }
      else if(strStrategy.compare("both") == 0) { png_options.block_split_strategy = 3; }
      else {
        ThrowException(Exception::TypeError(String::New("Wrong value for option 'block_split_strategy'")));
      }
    } else {
      //Wrong
      ThrowException(Exception::TypeError(String::New("Wrong type for option 'block_split_strategy'")));
      return false;
    }
  }
  return true;
}


Handle<Value> PNGDeflate(const Arguments& args) {
  HandleScope scope;
  
  if(args.Length() < 1 || !args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First argument must be a string")));
    return scope.Close(Undefined());
  }
  std::string imageName(*String::AsciiValue(args[0]->ToString()));

  if(args.Length() < 2 || !args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("First argument must be a string")));
    return scope.Close(Undefined());
  }
  std::string out_filename(*String::AsciiValue(args[1]->ToString()));

  ZopfliPNGOptions png_options;
  
  if(args.Length() >= 2 && args[2]->IsObject()) {
    Handle<Object> options = Handle<Object>::Cast(args[2]);
    if(!parseOptions(options, png_options)) {
      return scope.Close(Undefined());
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
  return scope.Close(Integer::New(error));
}
