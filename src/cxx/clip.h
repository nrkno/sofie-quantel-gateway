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

#ifndef QGW_CLIP
#define QGW_CLIP

#include <vector>
#include <map>
#include "qgw_util.h"

napi_value getClipData(napi_env env, napi_callback_info info);
napi_value searchClips(napi_env env, napi_callback_info info);
napi_value getFragments(napi_env env, napi_callback_info info);
napi_value cloneIfNeeded(napi_env env, napi_callback_info info);
napi_value deleteClip(napi_env env, napi_callback_info info);

void getClipDataExecute(napi_env env, void* data);
void getClipDataComplete(napi_env env, napi_status asyncStatus, void* data);

struct clipDataCarrier : carrier {
	int32_t clipID;
	std::vector<std::string> columnNames;
	std::vector<std::string> columnTypes;
	std::vector<std::string> values;
	~clipDataCarrier() { }
};

const std::string booleanName = "Boolean";
const std::string numberName = "Number";
const std::string dateName = "Date";

void searchClipsExecute(napi_env env, void* data);
void searchClipsComplete(napi_env env, napi_status asyncStatus, void* data);

const std::vector<std::string> columnNames = {
	"ClipID",
	"ClipGUID",
	"CloneId",
	"Completed",
	"Created",
	"Description",
	"Frames",
	"Owner",
	"PoolID",
	"Title",
};

const std::vector<std::string> clipIDonly = {
	"ClipID"
};

struct searchClipsCarrier : carrier {
	std::map<std::wstring, std::wstring> query;
	std::vector<std::string> colNames = columnNames;
	int32_t limit = 10;
	bool idOnly = false;
	std::vector<std::string> values;
	~searchClipsCarrier() { }
};

void getFragmentsExecute(napi_env env, void* data);
void getFragmentsComplete(napi_env env, napi_status asyncStatus, void* data);

struct getFragmentsCarrier : carrier {
	int32_t clipID;
	int32_t start = -1;
	int32_t finish = -1;
	Quentin::ServerFragments_var fragments = {};
	Quentin::ServerFragments_var sourceTCs = {};
	Quentin::ServerFragments_var refTCs = {};
	~getFragmentsCarrier() { }
};

void cloneIfNeededExecute(napi_env env, void* data);
void cloneIfNeededComplete(napi_env env, napi_status asyncStatus, void* data);

struct cloneIfNeededCarrier : carrier {
	int32_t clipID;
	int32_t poolID;
	bool highPriority = false;
	bool copyCreated;
	~cloneIfNeededCarrier() { }
};

void deletClipExecute(napi_env env, void* data);
void deleteClipComplete(napi_env env, napi_status asyncStatus, void* data);

struct deleteClipCarrier : carrier {
	int32_t clipID;
	bool deleted;
	~deleteClipCarrier() { }
};

#endif // QGW_CLIP
