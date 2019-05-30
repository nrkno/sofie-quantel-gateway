#ifndef QGW_PORT
#define QGW_PORT

#include "qgw_util.h"

extern int32_t portCounter;

napi_value createPlayPort(napi_env env, napi_callback_info info);
napi_value getPlayPortStatus(napi_env env, napi_callback_info info);
napi_value releasePort(napi_env env, napi_callback_info info);

napi_value loadPlayPort(napi_env env, napi_callback_info info);

#endif // QGW_PORT
