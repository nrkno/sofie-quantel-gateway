#include <Quentin.hh>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwchar>
#include <string>
#include <codecvt>
#include <locale>
#include "node_api.h"

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }
#define CHECK_STATUS if (checkStatus(env, status, __FILE__, __LINE__ - 1) != napi_ok) return nullptr
#define PASS_STATUS if (status != napi_ok) return status

#define NAPI_THROW_ERROR(msg) { \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

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

  int orbc = 0;
  CORBA::ORB_var local_orb = CORBA::ORB_init(orbc, nullptr);

  CORBA::Object_ptr ptr = local_orb->string_to_object(isaIOR);
  free(isaIOR);
  *zp = Quentin::ZonePortal::_narrow(ptr);
  *orb = local_orb;
}

napi_value testConnection(napi_env env, napi_callback_info info) {

  napi_status status;
  napi_value result;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;

  status = retrieveZonePortal(env, info, &orb, &zp);
  CHECK_STATUS;

  long zoneNumber = zp->getZoneNumber();
  status = napi_get_boolean(env, zoneNumber > 0, &result);
  CHECK_STATUS;

  orb->destroy();

  return result;
}

napi_value getZoneInfo(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  CORBA::WChar* zoneName;

  status = retrieveZonePortal(env, info, &orb, &zp);
  CHECK_STATUS;

  long zoneNumber = zp->getZoneNumber();
  zoneName = zp->getZoneName(zoneNumber);

  status = napi_create_object(env, &result);
  CHECK_STATUS;

  status = napi_create_string_utf8(env, "ZonePortal", NAPI_AUTO_LENGTH, &prop);
  CHECK_STATUS;
  status = napi_set_named_property(env, result, "type", prop);
  CHECK_STATUS;

  status = napi_create_int32(env, zoneNumber, &prop);
  CHECK_STATUS;
  status = napi_set_named_property(env, result, "zoneNumber", prop);
  CHECK_STATUS;

  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
  std::string zoneNameStr = utf8_conv.to_bytes(zoneName);

  status = napi_create_string_utf8(env, zoneNameStr.c_str(), NAPI_AUTO_LENGTH, &prop);
  CHECK_STATUS;
  status = napi_set_named_property(env, result, "zoneName", prop);
  CHECK_STATUS;

  orb->destroy();

  return result;
}

napi_value getServers(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, subprop, item;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;

  status = retrieveZonePortal(env, info, &orb, &zp);
  CHECK_STATUS;

  status = napi_create_array(env, &result);
  CHECK_STATUS;

  Quentin::Longs_var serverIDs = zp->getServers(true);
  for ( int x = 0 ; x < serverIDs->length() ; x++ ) {
    status = napi_create_object(env, &item);
    CHECK_STATUS;

    status = napi_create_string_utf8(env, "Server", NAPI_AUTO_LENGTH, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, item, "type", prop);
    CHECK_STATUS;

    status = napi_create_int64(env, serverIDs[x] > 0 ? serverIDs[x] : -serverIDs[x], &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, item, "ident", prop);
    CHECK_STATUS;

    if (serverIDs[x] > 0) { // server is o
      Quentin::Server server = zp->getServer(serverIDs[x]);
      Quentin::ServerInfo* serverInfo = server->getServerInfo();

      status = napi_get_boolean(env, serverInfo->down, &prop);
      CHECK_STATUS;
      status = napi_set_named_property(env, item, "down", prop);
      CHECK_STATUS;

      std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
      std::string serverNameStr = utf8_conv.to_bytes(serverInfo->name);

      status = napi_create_string_utf8(env, serverNameStr.c_str(), NAPI_AUTO_LENGTH, &prop);
      CHECK_STATUS;
      status = napi_set_named_property(env, item, "name", prop);
      CHECK_STATUS;

      status = napi_create_int32(env, serverInfo->numChannels, &prop);
      CHECK_STATUS;
      status = napi_set_named_property(env, item, "numChannels", prop);
      CHECK_STATUS;

      status = napi_create_array(env, &prop);
      CHECK_STATUS;
      for ( int y = 0 ; y < serverInfo->pools.length() ; y++ ) {
        status = napi_create_int32(env, serverInfo->pools[y], &subprop);
        CHECK_STATUS;
        status = napi_set_element(env, prop, y, subprop);
        CHECK_STATUS;
      }
      status = napi_set_named_property(env, item, "pools", prop);
      CHECK_STATUS;

      Quentin::WString_var portNames = server->getPortNames();
      status = napi_create_array(env, &prop);
      CHECK_STATUS;
      for ( int y = 0 ; y < portNames.length(); y++ ) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
        std::string portName = utf8_conv.to_bytes(portNames[y]);

        status = napi_create_string_utf8(env, portName.c_str(), NAPI_AUTO_LENGTH, &subprop);
        CHECK_STATUS;
        status = napi_set_element(env, prop, y, subprop);
        CHECK_STATUS;
      }
      status = napi_set_named_property(env, item, "portNames", prop);
      CHECK_STATUS;
    } else {
      status = napi_get_boolean(env, true, &prop);
      CHECK_STATUS;
      status = napi_set_named_property(env, item, "down", prop);
      CHECK_STATUS;
    }

    status = napi_set_element(env, result, x, item);
    CHECK_STATUS;
  }

  orb->destroy();
  return result;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

  napi_property_descriptor desc[] = {
    DECLARE_NAPI_METHOD("testConnection", testConnection),
    DECLARE_NAPI_METHOD("getZoneInfo", getZoneInfo),
    DECLARE_NAPI_METHOD("getServers", getServers)
  };
  status = napi_define_properties(env, exports, 3, desc);

  return exports;
}

NAPI_MODULE(sofie_quantel, Init)
