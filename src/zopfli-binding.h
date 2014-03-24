#ifndef _NODE_ZOPFLI_H_
#define _NODE_ZOPFLI_H_
#include <node.h>
#include <v8.h>
#include <string>
#include "nan.h"

namespace nodezopfli {

/*
 * struct used in the async versions, used to store data.
 */
template<class T> struct ZopfliRequest {
  ZopfliRequest(_NAN_METHOD_ARGS);
  ZopfliFormat format;
  ZopfliOptions zopfli_options;
  std::string input;
  char* resultdata;
  size_t resultsize;
  NanCallback *callback;
  const std::string* err;
};

class Base {
 protected:
  static void CallErrCallback(const v8::Handle<v8::Function>&,
                              const v8::Handle<v8::Value>&);
  static void CallCallback(const v8::Handle<v8::Function>&,
                           const v8::Handle<v8::Value>&,
                           const v8::Handle<v8::Value>&);
};

class CompressBinding : Base {
 protected:
  static void After(uv_work_t*);
  static void CallOkCallback(const v8::Handle<v8::Function>&,
                             const char* data, size_t size);
 public:
  static NAN_METHOD(Async);
  static NAN_METHOD(Sync);

 private:
  static void AsyncOperation(uv_work_t*);
};

}

#endif
