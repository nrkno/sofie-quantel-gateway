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

#ifndef QGW_CLONE
#define QGW_CLONE

#include "qgw_util.h"
#include <vector>

// Deprecated ... cloneInterZone is now a general clone operation
napi_value cloneIfNeeded(napi_env env, napi_callback_info info);

napi_value cloneInterZone(napi_env env, napi_callback_info info);
napi_value getCopyRemaining(napi_env env, napi_callback_info info);
napi_value getCopiesRemaining(napi_env env, napi_callback_info info);

void cloneIfNeededExecute(napi_env env, void* data);
void cloneIfNeededComplete(napi_env env, napi_status asyncStatus, void* data);

struct cloneIfNeededCarrier : carrier {
	int32_t clipID;
	int32_t poolID;
	bool highPriority = false;
	bool copyCreated;
	~cloneIfNeededCarrier() { }
};

void cloneInterZoneExecute(napi_env env, void* data);
void cloneInterZoneComplete(napi_env env, napi_status asyncStatus, void* data);

struct cloneInterZoneCarrier : carrier {
	int32_t zoneID = -1; // ZoneID of the remote zone where the source clip resides, negative for same zone
	int32_t clipID; // The clipID of the clip in the remote zone to be cloned
	int32_t poolID; // PoolID in the local destination zone where the clip it copied to
	int32_t priority = Quentin::Port::StandardPriority; // The priority of the transfer
	int32_t copyID; // Newly allocated clipID on the local destination zone
	bool history = true;
	bool copyCreated = true;
	~cloneInterZoneCarrier() { }
};

void getCopyRemainingExecute(napi_env env, void* data);
void getCopyRemainingComplete(napi_env env, napi_status asyncStatus, void* data);

struct getCopyRemainingCarrier : carrier {
	int32_t clipID;
	Quentin::CopyProgress progress = {};
	~getCopyRemainingCarrier() { }
};

void getCopiesRemainingExecute(napi_env env, void* data);
void getCopiesRemainingComplete(napi_env env, napi_status asyncStatus, void* data);

struct getCopiesRemainingCarrier : carrier {
	std::vector<Quentin::CopyProgress> progresses;
	~getCopiesRemainingCarrier() { }
};

#endif // QGW_CLONE
