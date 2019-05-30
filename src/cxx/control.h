#ifndef QGW_CONTROL
#define QGW_CONTROL

#include "qgw_util.h"

napi_value trigger(napi_env env, napi_callback_info info);
napi_value qJump(napi_env env, napi_callback_info info);
napi_value setJump(napi_env env, napi_callback_info info);

#endif // QGW_CONTROL
