#ifndef mHttpRequest_h__
#define mHttpRequest_h__

#include "mediaLib.h"

#ifdef GIT_BUILD // Define __M_FILE__
  #ifdef __M_FILE__
    #undef __M_FILE__
  #endif
  #define __M_FILE__ "xOTcXqYI4lfsiKz235L2XBG5Cy7vd73ao3pUmE5xAq/DbVP5knDpADu/yJ+cF9rcCwHxiqtC34ZoofJA"
#endif

struct mHttpRequest;

mFUNCTION(mHttpRequest_Create, OUT mPtr<mHttpRequest> *pHttpRequest, IN mAllocator *pAllocator, const mString &url);
mFUNCTION(mHttpRequest_Destroy, IN_OUT mPtr<mHttpRequest> *pHttpRequest);

mFUNCTION(mHttpRequest_AddHeadParameter, mPtr<mHttpRequest> &httpRequest, const mString &key, const mString &value);
mFUNCTION(mHttpRequest_AddHeader, mPtr<mHttpRequest> &httpRequest, const mString &header);
mFUNCTION(mHttpRequest_SetTimeout, mPtr<mHttpRequest> &httpRequest, const size_t timeoutMs);

mFUNCTION(mHttpRequest_Send, mPtr<mHttpRequest> &httpRequest);
mFUNCTION(mHttpRequest_Send, mPtr<mHttpRequest> &httpRequest, const std::function<mResult(const size_t downloadedSize)> &callback);

// If a successful request has been performed already: Retrieve size of the response.
// Can also be called the request has been sent. Will attempt to retrieve the `Content-Length` header parameter.
mFUNCTION(mHttpRequest_GetResponseSize, mPtr<mHttpRequest> &httpRequest, OUT size_t *pBytes);

mFUNCTION(mHttpRequest_GetResponseBytes, mPtr<mHttpRequest> &httpRequest, OUT uint8_t *pBuffer, const size_t bufferSize);
mFUNCTION(mHttpRequest_GetResponseString, mPtr<mHttpRequest> &httpRequest, OUT mString *pResponse);

mFUNCTION(mHttpRequest_GetResponseContentType, mPtr<mHttpRequest> &httpRequest, OUT mString *pContentType);

#endif // mHttpRequest_h__
