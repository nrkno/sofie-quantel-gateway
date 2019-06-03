#ifndef QGW_CONTROL
#define QGW_CONTROL

#include "qgw_util.h"

napi_value trigger(napi_env env, napi_callback_info info);
napi_value qJump(napi_env env, napi_callback_info info);
napi_value setJump(napi_env env, napi_callback_info info);

void triggerExecute(napi_env env, void* data);
void triggerComplete(napi_env env, napi_status asyncStatus, void* data);

struct triggerCarrier : carrier {
	int32_t serverID;
	std::string portName;
	int32_t trigger;
	int32_t offset = -1;
	~triggerCarrier() { }
};

void jumpExecute(napi_env env, void* data);
void jumpComplete(napi_env env, napi_status asyncStatus, void* data);

struct jumpCarrier : carrier {
	int32_t serverID;
	std::string portName;
	int32_t offset = 0;
	bool hardJump = true;
	~jumpCarrier() { }
};

napi_value coreJump(napi_env env, napi_callback_info info, bool hardJump);

#endif // QGW_CONTROL
