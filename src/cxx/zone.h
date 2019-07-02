/*
	 Copyright (c) 2019 Norsk rikskringkasting AS (NRK)

	 This file is part of Sofie: The Modern TV News Studio Automation
	 System (Quantel gateway)

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License along
	 with this program; if not, write to the Free Software Foundation, Inc.,
	 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef QGW_ZONE
#define QGW_ZONE

#include "qgw_util.h"
#include <vector>

napi_value testConnection(napi_env env, napi_callback_info info);
napi_value listZones(napi_env env, napi_callback_info info);
napi_value getServers(napi_env env, napi_callback_info info);
napi_value getFormatInfo(napi_env env, napi_callback_info info);
napi_value cloneInterZone(napi_env env, napi_callback_info info);

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

void cloneInterZoneExecute(napi_env env, void* data);
void cloneInterZoneComplete(napi_env env, napi_status asyncStatus, void* data);

struct cloneInterZoneCarrier : carrier {
	int32_t zoneID; // ZoneID of the remote zone where the source clip resides
	int32_t clipID; // The clipID of the clip in the remote zone to be cloned
	int32_t poolID; // PoolID in the local destination zone where the clip it copied to
	int32_t priority = Quentin::Port::StandardPriority; // The priority of the transfer
	int32_t copyID; // Newly allocated clipID on the local destination zone
	~cloneInterZoneCarrier() { }
};

#endif // QGW_ZONE
