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
