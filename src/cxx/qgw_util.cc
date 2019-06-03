#include "qgw_util.h"

napi_status checkStatus(napi_env env, napi_status status,
  const char* file, uint32_t line) {

  napi_status infoStatus, throwStatus;
  const napi_extended_error_info *errorInfo;

  if (status == napi_ok) {
    // printf("Received status OK.\n");
    return status;
  }

  infoStatus = napi_get_last_error_info(env, &errorInfo);
  assert(infoStatus == napi_ok);
  printf("NAPI error in file %s on line %i. Error %i: %s\n", file, line,
    errorInfo->error_code, errorInfo->error_message);

  if (status == napi_pending_exception) {
    printf("NAPI pending exception. Engine error code: %i\n", errorInfo->engine_error_code);
    return status;
  }

  char errorCode[20];
  sprintf(errorCode, "%d", errorInfo->error_code);
  throwStatus = napi_throw_error(env, errorCode, errorInfo->error_message);
  assert(throwStatus == napi_ok);

  return napi_pending_exception; // Expect to be cast to void
}

napi_status retrieveZonePortal(napi_env env, napi_callback_info info, CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp) {
  napi_status status;
  char* isaIOR = nullptr;
  size_t iorLen = 0;

  size_t argc = 1;
  napi_value argv[1];
  status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  PASS_STATUS;

  if (argc < 1) {
    printf("Connection test must be provided with a IOR reference to an ISA server.");
    return napi_string_expected;
  }
  status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
  PASS_STATUS;
  isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
  status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
  PASS_STATUS;

  const char* options[][2] = { { "traceLevel", "1" }, { 0, 0 } };
  int orbc = 0;
  CORBA::ORB_var local_orb = CORBA::ORB_init(orbc, nullptr, "omniORB4", options);

  CORBA::Object_ptr ptr = local_orb->string_to_object(isaIOR);
  free(isaIOR);
	*zp = Quentin::ZonePortal::_unchecked_narrow(ptr);
  *orb = local_orb;

	return napi_ok;
}

napi_status resolveZonePortal(char* ior, CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp) {
  const char* options[][2] = { { "traceLevel", "1" }, { 0, 0 } };
  int orbc = 0;
  CORBA::ORB_var local_orb = CORBA::ORB_init(orbc, nullptr, "omniORB4", options);

  CORBA::Object_ptr ptr = local_orb->string_to_object(ior);
	*zp = Quentin::ZonePortal::_unchecked_narrow(ptr);
  *orb = local_orb;

	return napi_ok;
}

std::string formatTimecode(Quentin::Timecode tc) {
	int32_t HH, hh, MM, mm, SS, ss, FF, ff;
	bool drop = (tc & 0x40000000) != 0;
	char* tcstr = (char*) malloc(12 * sizeof(char));
	HH = (tc >> 28) & 0x03;
	hh = (tc >> 24) & 0x0f;
	MM = (tc >> 20) & 0x07;
	mm = (tc >> 16) & 0x0f;
	SS = (tc >> 12) & 0x07;
	ss = (tc >>  8) & 0x0f;
	FF = (tc >>  4) & 0x07;
	ff = (tc >>  0) & 0x0f;
	snprintf(tcstr, 12, "%i%i:%i%i:%i%i%s%i%i", HH, hh, MM, mm, SS, ss,
    drop ? ";" : ":", FF, ff);
	std::string result(tcstr);
	free(tcstr);
	return result;
}

napi_status convertToDate(napi_env env, std::string date, napi_value *nodeDate) {
	napi_status status;
	napi_value global, dateObj, integerDate;

	status = napi_get_global(env, &global);
	PASS_STATUS;

	status = napi_get_named_property(env, global, "Date", &dateObj);
	PASS_STATUS;
	status = napi_create_int64(env, std::stoll(date), &integerDate);
	PASS_STATUS;

	napi_value argv[1] = { integerDate };
	status = napi_new_instance(env, dateObj, 1, argv, nodeDate);
	PASS_STATUS;

	return napi_ok;
}

void tidyCarrier(napi_env env, carrier* c) {
  napi_status status;
  if (c->passthru != nullptr) {
    status = napi_delete_reference(env, c->passthru);
    FLOATING_STATUS;
  }
  if (c->_request != nullptr) {
    status = napi_delete_async_work(env, c->_request);
    FLOATING_STATUS;
  }
  // printf("Tidying carrier %p %p\n", c->passthru, c->_request);
  delete c;
}

int32_t rejectStatus(napi_env env, carrier* c, char* file, int32_t line) {
  if (c->status != QGW_SUCCESS) {
    napi_value errorValue, errorCode, errorMsg;
    napi_status status;
    if (c->status < QGW_ERROR_START) {
      const napi_extended_error_info *errorInfo;
      status = napi_get_last_error_info(env, &errorInfo);
      FLOATING_STATUS;
      c->errorMsg = std::string(
        (errorInfo->error_message != nullptr) ? errorInfo->error_message : "(no message)");
    }
    char* extMsg = (char *) malloc(sizeof(char) * c->errorMsg.length() + 200);
    sprintf(extMsg, "In file %s on line %i, found error: %s", file, line, c->errorMsg.c_str());
    char errorCodeChars[20];
    sprintf(errorCodeChars, "%d", c->status);
    status = napi_create_string_utf8(env, errorCodeChars,
      NAPI_AUTO_LENGTH, &errorCode);
    FLOATING_STATUS;
    status = napi_create_string_utf8(env, extMsg, NAPI_AUTO_LENGTH, &errorMsg);
    FLOATING_STATUS;
    status = napi_create_error(env, errorCode, errorMsg, &errorValue);
    FLOATING_STATUS;
    status = napi_reject_deferred(env, c->_deferred, errorValue);
    FLOATING_STATUS;

    delete[] extMsg;
    tidyCarrier(env, c);
  }
  return c->status;
}
