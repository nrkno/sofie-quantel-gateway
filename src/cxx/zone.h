#ifndef QGW_ZONE
#define QGW_ZONE

#include "qgw_util.h"

napi_value testConnection(napi_env env, napi_callback_info info);
napi_value listZones(napi_env env, napi_callback_info info);
napi_value getDefaultZoneInfo(napi_env env, napi_callback_info info);
napi_value getServers(napi_env env, napi_callback_info info);

#endif // QGW_ZONE
