#ifndef QGW_CLIP
#define QGW_CLIP

#include <vector>
#include "qgw_util.h"

napi_value getClipData(napi_env env, napi_callback_info info);
napi_value searchClips(napi_env env, napi_callback_info info);
napi_value getFragments(napi_env env, napi_callback_info info);
napi_value cloneIfNeeded(napi_env env, napi_callback_info info);

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

#endif // QGW_CLIP
