#ifndef QGW_PORT
#define QGW_PORT

#include "qgw_util.h"
#include <vector>

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

void getPlayPortExecute(napi_env env, void* data);
void getPlayPortComplete(napi_env env, napi_status asyncStatus, void* data);

struct playPortStatusCarrier : carrier {
	int32_t serverID;
	std::string portName;
  std::string refTime;
	std::string portTime;
	int32_t portNumber;
	double speed;
	int64_t offset;
	std::string statusFlags;
	int64_t endOfData;
	int64_t framesUnused;
	std::string outputTime;
	std::vector<int32_t> channels;
	~playPortStatusCarrier() { }
};

void releasePortExecute(napi_env env, void* data);
void releasePortComplete(napi_env env, napi_status asyncStatus, void* data);

struct releasePortCarrier : carrier {
	int32_t serverID;
	std::string portName;
	~releasePortCarrier() { }
};

void loadPlayPortExecute(napi_env env, void* data);
void loadPlayPortComplete(napi_env env, napi_status asyncStatus, void* data);

struct loadPlayPortCarrier : carrier {
	int32_t serverID;
	std::string portName;
	int32_t offset = 0;
	Quentin::ServerFragments fragments = {};
	~loadPlayPortCarrier() { }
};

#endif // QGW_PORT
