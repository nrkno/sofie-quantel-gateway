#include "qgw_util.h"
#include "zone.h"
#include "clip.h"
#include "port.h"
#include "control.h"
#include "thumbs.h"
#include "test_server.h"

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

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value start, stop, jump, transition;
  status = napi_create_int32(env, START, &start);
	CHECK_STATUS;
  status = napi_create_int32(env, STOP, &stop);
	CHECK_STATUS;
  status = napi_create_int32(env, JUMP, &jump);
	CHECK_STATUS;
  status = napi_create_int32(env, TRANSITION, &transition);
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
    DECLARE_NAPI_METHOD("trigger", trigger),
    DECLARE_NAPI_METHOD("jump", qJump),
    DECLARE_NAPI_METHOD("setJump", setJump),
		DECLARE_NAPI_METHOD("getThumbnailSize", getThumbnailSize),
		DECLARE_NAPI_METHOD("requestThumbnails", requestThumbnails),
		DECLARE_NAPI_METHOD("cloneIfNeeded", cloneIfNeeded),
		DECLARE_NAPI_METHOD("deleteClip", deleteClip),
		DECLARE_NAPI_METHOD("timecodeFromBCD", timecodeFromBCD),
		DECLARE_NAPI_METHOD("timecodeToBCD", timecodeToBCD), // 20
		DECLARE_NAPI_METHOD("runServer", runServer),
    { "START", nullptr, nullptr, nullptr, nullptr, start, napi_enumerable, nullptr },
    { "STOP", nullptr, nullptr, nullptr, nullptr, stop, napi_enumerable, nullptr },
    { "JUMP", nullptr, nullptr, nullptr, nullptr, jump, napi_enumerable, nullptr },
    { "TRANSITION", nullptr, nullptr, nullptr, nullptr, transition, napi_enumerable, nullptr },
  };
  status = napi_define_properties(env, exports, 25, desc);
	CHECK_STATUS;

  return exports;
}

NAPI_MODULE(quantel_gateway, Init)
