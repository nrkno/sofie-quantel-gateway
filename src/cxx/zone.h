#ifndef QGW_ZONE
#define QGW_ZONE

#include "qgw_util.h"

napi_value testConnection(napi_env env, napi_callback_info info);
napi_value listZones(napi_env env, napi_callback_info info);
napi_value getDefaultZoneInfo(napi_env env, napi_callback_info info);
napi_value getServers(napi_env env, napi_callback_info info);

void testConnectionExecute(napi_env env, void* data);
void testConnectionComplete(napi_env env, napi_status asyncStatus, void* data);

struct testConnectionCarrier : carrier {
	char* isaIOR = nullptr;
	long zoneNumber = -1;
	~testConnectionCarrier() {
		if (isaIOR != nullptr) {
			free(isaIOR);
	 	}
	}
};

#endif // QGW_ZONE
