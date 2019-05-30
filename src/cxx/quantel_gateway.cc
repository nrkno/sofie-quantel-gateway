#include "qgw_util.h"
#include "zone.h"
#include "clip.h"
#include "port.h"
#include "control.h"
#include "thumbs.h"

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value start, stop, jump, transition;
  status = napi_create_int32(env, START, &start);
  status = napi_create_int32(env, STOP, &stop);
  status = napi_create_int32(env, JUMP, &jump);
  status = napi_create_int32(env, TRANSITION, &transition);

  napi_property_descriptor desc[] = {
    DECLARE_NAPI_METHOD("testConnection", testConnection),
		DECLARE_NAPI_METHOD("listZones", listZones),
    DECLARE_NAPI_METHOD("getDefaultZoneInfo", getDefaultZoneInfo),
    DECLARE_NAPI_METHOD("getServers", getServers),
    DECLARE_NAPI_METHOD("createPlayPort", createPlayPort),
    DECLARE_NAPI_METHOD("getPlayPortStatus", getPlayPortStatus),
    DECLARE_NAPI_METHOD("releasePort", releasePort),
		DECLARE_NAPI_METHOD("getClipData", getClipData),
    DECLARE_NAPI_METHOD("getFragments", getFragments),
		DECLARE_NAPI_METHOD("searchClips", searchClips),
    DECLARE_NAPI_METHOD("loadPlayPort", loadPlayPort),
    DECLARE_NAPI_METHOD("trigger", trigger),
    DECLARE_NAPI_METHOD("jump", qJump),
    DECLARE_NAPI_METHOD("setJump", setJump),
		DECLARE_NAPI_METHOD("getThumbnailSize", getThumbnailSize),
		DECLARE_NAPI_METHOD("requestThumbnails", requestThumbnails),
		DECLARE_NAPI_METHOD("cloneIfNeeded", cloneIfNeeded),
    { "START", nullptr, nullptr, nullptr, nullptr, start, napi_enumerable, nullptr },
    { "STOP", nullptr, nullptr, nullptr, nullptr, stop, napi_enumerable, nullptr },
    { "JUMP", nullptr, nullptr, nullptr, nullptr, jump, napi_enumerable, nullptr },
    { "TRANSITION", nullptr, nullptr, nullptr, nullptr, transition, napi_enumerable, nullptr },
  };
  status = napi_define_properties(env, exports, 21, desc);

  return exports;
}

NAPI_MODULE(quantel_gateway, Init)
