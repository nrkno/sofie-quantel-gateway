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

#include "qgw_util.h"
#include "zone.h"
#include "clip.h"
#include "port.h"
#include "control.h"
#include "clone.h"
#include "thumbs.h"
#include "test_server.h"
#include <omniORB4/omniORB.h>

napi_value timecodeFromBCD(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value result;
	uint32_t timecode;

	size_t argc = 1;
	napi_value argv[1];
	status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	CHECK_STATUS;

	if (argc != 1) {
		NAPI_THROW_ERROR("Timecode conversion requires a single numberical value.");
	}

	status = napi_get_value_uint32(env, argv[0], &timecode);
	CHECK_STATUS;

	status = napi_create_string_utf8(env, formatTimecode(timecode).c_str(),
		NAPI_AUTO_LENGTH, &result);
	CHECK_STATUS;

	return result;
}

napi_value timecodeToBCD(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value result;
	char tcChars[12];

	size_t argc = 1;
	napi_value argv[1];
	status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	CHECK_STATUS;

	if (argc != 1) {
		NAPI_THROW_ERROR("Timecode conversion requires a single string value.");
	}

	status = napi_get_value_string_utf8(env, argv[0], tcChars, 12, nullptr);
	CHECK_STATUS;

	status = napi_create_uint32(env, timecodeFromString(std::string(tcChars)), &result);
	CHECK_STATUS;

	return result;
}

CORBA::Boolean commFailureHandler (void* cookie, CORBA::ULong retries, const CORBA::COMM_FAILURE& ex)
{
   printf("comm failure handler called.\n");
   return 0;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value start, stop, jump, transition, standard, high;
  status = napi_create_int32(env, START, &start);
	CHECK_STATUS;
  status = napi_create_int32(env, STOP, &stop);
	CHECK_STATUS;
  status = napi_create_int32(env, JUMP, &jump);
	CHECK_STATUS;
  status = napi_create_int32(env, TRANSITION, &transition);
	CHECK_STATUS;
	status = napi_create_int32(env, Quentin::Port::StandardPriority, &standard);
	CHECK_STATUS;
	status = napi_create_int32(env, Quentin::Port::HighPriority, &high);
	CHECK_STATUS;

  napi_property_descriptor desc[] = {
    DECLARE_NAPI_METHOD("testConnection", testConnection),
		DECLARE_NAPI_METHOD("listZones", listZones),
    DECLARE_NAPI_METHOD("getServers", getServers),
    DECLARE_NAPI_METHOD("createPlayPort", createPlayPort),
    DECLARE_NAPI_METHOD("getPlayPortStatus", getPlayPortStatus),
    DECLARE_NAPI_METHOD("releasePort", releasePort),
		DECLARE_NAPI_METHOD("wipe", wipe),
		DECLARE_NAPI_METHOD("getClipData", getClipData),
    DECLARE_NAPI_METHOD("getFragments", getFragments),
		DECLARE_NAPI_METHOD("searchClips", searchClips), // 10
    DECLARE_NAPI_METHOD("loadPlayPort", loadPlayPort),
		DECLARE_NAPI_METHOD("getPortFragments", getPortFragments),
    DECLARE_NAPI_METHOD("trigger", trigger),
    DECLARE_NAPI_METHOD("jump", qJump),
    DECLARE_NAPI_METHOD("setJump", setJump),
		DECLARE_NAPI_METHOD("getThumbnailSize", getThumbnailSize),
		DECLARE_NAPI_METHOD("requestThumbnails", requestThumbnails),
		DECLARE_NAPI_METHOD("cloneIfNeeded", cloneIfNeeded),
		DECLARE_NAPI_METHOD("deleteClip", deleteClip),
		DECLARE_NAPI_METHOD("timecodeFromBCD", timecodeFromBCD), // 20
		DECLARE_NAPI_METHOD("timecodeToBCD", timecodeToBCD),
		DECLARE_NAPI_METHOD("getFormatInfo", getFormatInfo),
		DECLARE_NAPI_METHOD("getPortProperties", getPortProperties),
		DECLARE_NAPI_METHOD("cloneInterZone", cloneInterZone),
		DECLARE_NAPI_METHOD("getCopyRemaining", getCopyRemaining),
		DECLARE_NAPI_METHOD("getCopiesRemaining", getCopiesRemaining),
		DECLARE_NAPI_METHOD("runServer", runServer),
		DECLARE_NAPI_METHOD("closeServer", closeServer),
		DECLARE_NAPI_METHOD("performWork", performWork),
		DECLARE_NAPI_METHOD("deactivatePman", deactivatePman), 
		DECLARE_NAPI_METHOD("destroyOrb", destroyOrb),
    { "START", nullptr, nullptr, nullptr, nullptr, start, napi_enumerable, nullptr },
    { "STOP", nullptr, nullptr, nullptr, nullptr, stop, napi_enumerable, nullptr }, // 30
    { "JUMP", nullptr, nullptr, nullptr, nullptr, jump, napi_enumerable, nullptr },
    { "TRANSITION", nullptr, nullptr, nullptr, nullptr, transition, napi_enumerable, nullptr },
		{ "STANDARD", nullptr, nullptr, nullptr, nullptr, standard, napi_enumerable, nullptr},
		{ "HIGH", nullptr, nullptr, nullptr, nullptr, high, napi_enumerable, nullptr},
  };
  status = napi_define_properties(env, exports, 37, desc);
	CHECK_STATUS;

  omniORB::installCommFailureExceptionHandler(0, commFailureHandler);

  return exports;
}

NAPI_MODULE(quantel_gateway, Init)
