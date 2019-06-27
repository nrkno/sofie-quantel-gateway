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

#ifndef QGW_PORT
#define QGW_PORT

#include "qgw_util.h"
#include <vector>
#include <map>

extern int32_t portCounter;

napi_value createPlayPort(napi_env env, napi_callback_info info);
napi_value getPlayPortStatus(napi_env env, napi_callback_info info);
napi_value releasePort(napi_env env, napi_callback_info info);
napi_value wipe(napi_env env, napi_callback_info info);

napi_value loadPlayPort(napi_env env, napi_callback_info info);
napi_value getPortFragments(napi_env env, napi_callback_info info);

napi_value getPortProperties(napi_env env, napi_callback_info info);

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
	std::string videoFormat = "";
	~playPortStatusCarrier() { }
};

void releasePortExecute(napi_env env, void* data);
void releasePortComplete(napi_env env, napi_status asyncStatus, void* data);

struct releasePortCarrier : carrier {
	int32_t serverID;
	std::string portName;
	~releasePortCarrier() { }
};

void wipeExecute(napi_env env, void* data);
void wipeComplete(napi_env env, napi_status asyncStatus, void* data);

struct wipeCarrier : carrier {
	int32_t serverID;
	std::string portName;
	int32_t start = 0;
	int32_t frames = 0x7fffffff;
	bool wiped = false;
	~wipeCarrier() { }
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

void getPortFragmentsExecute(napi_env env, void* data);
void getPortFragmentsComplete(napi_env env, napi_status asyncStatus, void* data);

struct portFragmentsCarrier : carrier {
	int32_t serverID;
	std::string portName;
	int32_t start = 0;
	int32_t finish = 0x7fffffff;
	Quentin::ServerFragments_var fragments = {};
	~portFragmentsCarrier() { }
};

void getPortPropertiesExecute(napi_env env, void* data);
void getPortPropertiesComplete(napi_env env, napi_status asyncStatus, void* data);

struct portPropertiesCarrier : carrier {
	int32_t serverID;
	std::string portName;
	std::map<std::string, std::string> properties;
	~portPropertiesCarrier() { }
};

#endif // QGW_PORT
