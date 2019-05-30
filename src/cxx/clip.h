#ifndef QGW_CLIP
#define QGW_CLIP

#include "qgw_util.h"

napi_value getClipData(napi_env env, napi_callback_info info);
napi_value searchClips(napi_env env, napi_callback_info info);
napi_value getFragments(napi_env env, napi_callback_info info);
napi_value cloneIfNeeded(napi_env env, napi_callback_info info);

#endif // QGW_CLIP
