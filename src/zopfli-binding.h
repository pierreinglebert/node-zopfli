#ifndef _NODE_ZOPFLI_H_
#define _NODE_ZOPFLI_H_
#include <node.h>
#include <v8.h>
#include <string>
#include "nan.h"

#define _THROW(type, errmsg) \
  NanThrowError(_NAN_ERROR(type, errmsg));

NAN_INLINE void GetOptionIfExists(
    const v8::Local<v8::Object> optionsObj
  , const v8::Handle<v8::String> opt
  , bool* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    *def = optionsObj->Get(opt)->BooleanValue();
  }
}

NAN_INLINE void GetOptionIfExists(
    const v8::Local<v8::Object> optionsObj
  , const v8::Handle<v8::String> opt
  , uint32_t* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    *def = optionsObj->Get(opt)->Uint32Value();
  }
}

NAN_INLINE void GetOptionIfExists(
    const v8::Local<v8::Object> optionsObj
  , const v8::Handle<v8::String> opt
  , int32_t* def
) {
  if (!optionsObj.IsEmpty() && optionsObj->Has(opt)) {
    *def = optionsObj->Get(opt)->Int32Value();
  }
}

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


class CompressBinding : public NanAsyncWorker {
 protected:
  static void After(uv_work_t*);
  static void CallOkCallback(NanCallback*,
                             const char* data, size_t size);
 public:
  static NAN_METHOD(Async);
  static NAN_METHOD(Sync);
};

}

#endif
