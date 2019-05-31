#ifndef QGW_PORT
#define QGW_PORT

#include "qgw_util.h"

extern int32_t portCounter;

napi_value createPlayPort(napi_env env, napi_callback_info info);
napi_value getPlayPortStatus(napi_env env, napi_callback_info info);
napi_value releasePort(napi_env env, napi_callback_info info);

napi_value loadPlayPort(napi_env env, napi_callback_info info);

void createPlayPortExecute(napi_env env, void* data);
void createPlayPortComplete(napi_env env, napi_status asyncStatus, void* data);

struct createPlayPortCarrier : carrier {
	int32_t serverID;
  int32_t channelNo;
	std::string portName;
	bool audioOnly = false;
	bool assigned = false;
	int32_t portID = -1;
	~createPlayPortCarrier() { }
};

#endif // QGW_PORT
