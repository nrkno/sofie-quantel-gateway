#ifndef QGW_ZONE
#define QGW_ZONE

#include "qgw_util.h"
#include <vector>

napi_value testConnection(napi_env env, napi_callback_info info);
napi_value listZones(napi_env env, napi_callback_info info);
napi_value getServers(napi_env env, napi_callback_info info);
napi_value getFormatInfo(napi_env env, napi_callback_info info);

void testConnectionExecute(napi_env env, void* data);
void testConnectionComplete(napi_env env, napi_status asyncStatus, void* data);

struct testConnectionCarrier : carrier {
	long zoneNumber = -1;
	~testConnectionCarrier() {}
};

void listZonesExecute(napi_env env, void* data);
void listZonesComplete(napi_env env, napi_status asyncStatus, void* data);

struct listZonesCarrier : carrier {
	Quentin::Longs_var zoneIDs;
	CORBA::WChar** zoneNames = nullptr;
	CORBA::Boolean* remotes = nullptr;
	~listZonesCarrier() {
	  if (zoneNames != nullptr) {
			free(zoneNames);
		}
		if (remotes != nullptr) {
			free(remotes);
		}
	}
};

void getServersExecute(napi_env env, void* data);
void getServersComplete(napi_env env, napi_status asyncStatus, void* data);

struct serverDetails {
	long ident;
	bool down = true;
	std::string name;
	long numChannels;
	std::vector<long> pools;
  std::vector<std::string> portNames;
	std::vector<std::string> chanPorts;
	~serverDetails() {
		// printf("Server details destructor called.\n");
	}
};

struct getServersCarrier : carrier {
	std::vector<serverDetails*> servers;
	~getServersCarrier() {
		// printf("Clearing getServers carrier\n");
		for ( auto it = servers.cbegin() ; it != servers.cend() ; it++ ) {
			delete *it;
		}
		servers.clear();
	}
};

void getFormatInfoExecute(napi_env env, void* data);
void getFormatInfoComplete(napi_env env, napi_status asyncStatus, void* data);

struct formatInfoCarrier : carrier {
	Quentin::FormatCode formatNumber;
	Quentin::FormatInfo_var info;
	~formatInfoCarrier() { }
};

#endif // QGW_ZONE
