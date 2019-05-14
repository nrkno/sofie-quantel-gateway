#include <Quentin.hh>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwchar>
#include <string>
#include <codecvt>
#include <locale>
#include "node_api.h"

#define OMNI_UNLOADABLE_STUBS

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }
#define CHECK_STATUS if (checkStatus(env, status, __FILE__, __LINE__ - 1) != napi_ok) { orb->destroy(); return nullptr; }
#define PASS_STATUS if (status != napi_ok) return status

#define NAPI_THROW_ERROR(msg) { \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_ORB_DESTROY(msg) { \
  orb->destroy(); \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_CORBA_EXCEPTION(ex) { \
  orb->destroy(); \
  char errorMsg[256]; \
  sprintf(errorMsg, "Exception thrown from CORBA subsystem: %s", ex._name()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_FATAL_EXCEPTION(fe) { \
  orb->destroy(); \
  char errorMsg[512]; \
  sprintf(errorMsg, "Omni ORB fatal exception thrown at line %i of %s: %s", fe.line(), fe.file(), fe.errmsg()); \
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

  const char* options[][2] = { { "traceLevel", "10" }, { 0, 0 } };
  int orbc = 0;
  CORBA::ORB_var local_orb = CORBA::ORB_init(orbc, nullptr, "omniORB4", options);

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
  Quentin::Server_ptr server;
  Quentin::ServerInfo* serverInfo;
  Quentin::WStrings_var portNames;
  std::string portName, serverNameStr;

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
      server = zp->getServer(serverIDs[x]);
      serverInfo = server->getServerInfo();

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

      portNames = server->getPortNames();
      status = napi_create_array(env, &prop);
      CHECK_STATUS;
      for ( int y = 0 ; y < portNames->length(); y++ ) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
        portName = utf8_conv.to_bytes(portNames[y]);

        status = napi_create_string_utf8(env, portName.c_str(), NAPI_AUTO_LENGTH, &subprop);
        CHECK_STATUS;
        status = napi_set_element(env, prop, y, subprop);
        CHECK_STATUS;
      }
      status = napi_set_named_property(env, item, "portNames", prop);
      CHECK_STATUS;

      // Quentin::WStrings_var chanPorts = server->getChanPorts();
      // status = napi_create_array(env, &prop);
      // CHECK_STATUS;
      // for ( int y = 0 ; y < chanPorts->length(); y++ ) {
        // if (wcslen(chanPorts[y]) > 0) {
        //   std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
        //   std::string chanPort = utf8_conv.to_bytes(chanPorts[y]);
        //   status = napi_create_string_utf8(env, chanPort.c_str(), NAPI_AUTO_LENGTH, &subprop);
        //   CHECK_STATUS;
        //   status = napi_set_element(env, prop, y, subprop);
        //   CHECK_STATUS;
        // }
      // }
      // status = napi_set_named_property(env, item, "chanPorts", prop);
      // CHECK_STATUS;

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

#define START 42
#define STOP 43

int32_t portCounter = 1;

napi_value createPlayPort(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID, channelNo;
  char* portName;
  size_t portNameLen;
  bool audioOnly;
  int32_t portID = portCounter++;

  try {
    status = retrieveZonePortal(env, info, &orb, &zp);
    CHECK_STATUS;

    size_t argc = 2;
    napi_value argv[2];
    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_STATUS;

    if (argc < 2) {
      NAPI_THROW_ORB_DESTROY("Options object with server ID, port name and channel must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with server ID, port name and channel.");
    }

    status = napi_create_object(env, &result);
    status = napi_create_string_utf8(env, "PortInfo", NAPI_AUTO_LENGTH, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "type", prop);
    CHECK_STATUS;

    options = argv[1];
    status = napi_get_named_property(env, options, "serverID", &prop);
    CHECK_STATUS;
    status = napi_get_value_int32(env, prop, &serverID);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "serverID", prop);
    CHECK_STATUS;

    status = napi_get_named_property(env, options, "channelNo", &prop);
    CHECK_STATUS;
    status = napi_get_value_int32(env, prop, &channelNo);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "channelNo", prop);
    CHECK_STATUS;

    status = napi_has_named_property(env, options, "audioOnly", &audioOnly);
    CHECK_STATUS;
    if (audioOnly) {
      status = napi_get_named_property(env, options, "audioOnly", &prop);
      CHECK_STATUS;
      status = napi_get_value_bool(env, prop, &audioOnly);
      CHECK_STATUS;
    }
    status = napi_set_named_property(env, result, "audioOnly", prop);
    CHECK_STATUS;

    status = napi_get_named_property(env, options, "portName", &prop);
    CHECK_STATUS;
    status = napi_get_value_string_utf8(env, prop, nullptr, 0, &portNameLen);
    CHECK_STATUS;
    portName = (char*) malloc((portNameLen + 1) * sizeof(char));
    status = napi_get_value_string_utf8(env, prop, portName, portNameLen + 1, &portNameLen);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "portName", prop);
    CHECK_STATUS;

    status = napi_has_named_property(env, options, "portID", &isArray);
    CHECK_STATUS;
    if (isArray) {
      status = napi_get_named_property(env, options, "portID", &prop);
      CHECK_STATUS;
      status = napi_get_value_int32(env, prop, &portID);
      CHECK_STATUS;
    }
    status = napi_create_int32(env, portID, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "portID", prop);
    CHECK_STATUS;


    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), portID);
    if (port == nullptr) {
      NAPI_THROW_ORB_DESTROY("Unable to create port. It is likely that all ports are assigned.");
    }

    CORBA::Boolean assigned = port->assignChannel(channelNo, audioOnly ? 1 : 0);
    status = napi_get_boolean(env, assigned, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "assigned", prop);
    CHECK_STATUS;

    CORBA::Boolean playing = port->setMode(Quentin::Port::PortMode::playing);
    if (!playing) {
      NAPI_THROW_ORB_DESTROY("Failed to set playing mode for port.");
    }

    // Quentin::ServerFragments_var fragments = zp->getAllFragments(2);
    // port->load(0, fragments);
    // port->actionAtTrigger(START, Quentin::Port::trActStart);
    // port->actionAtTrigger(STOP, Quentin::Port::trActStop);

    // port->setTrigger(START, Quentin::Port::trModeNow, 0);
    // port->setTrigger(STOP, Quentin::Port::trModeOffset, 300);

    // Quentin::PortListener::PlayPortStatus * gps = &port->getStatus().playStatus();
    //
    // printf("Playing at speed %f\n", gps->speed);
    //
    // port->release();
  }
  catch(CORBA::SystemException& ex) {
    NAPI_THROW_CORBA_EXCEPTION(ex);
  }
  catch(CORBA::Exception& ex) {
    NAPI_THROW_CORBA_EXCEPTION(ex);
  }
  catch(omniORB::fatalException& fe) {
    NAPI_THROW_FATAL_EXCEPTION(fe);
  }

  orb->destroy();
  return result;
}

napi_value getPlayPortStatus(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID;
  char* portName;
  size_t portNameLen;
  Quentin::WStrings_var portNames;

  try {
    status = retrieveZonePortal(env, info, &orb, &zp);
    CHECK_STATUS;

    size_t argc = 2;
    napi_value argv[2];
    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_STATUS;

    if (argc < 2) {
      NAPI_THROW_ORB_DESTROY("Options object with server ID, port name and channel must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with server ID, port name and channel.");
    }

    status = napi_create_object(env, &result);
    CHECK_STATUS;
    status = napi_create_string_utf8(env, "PortStatus", NAPI_AUTO_LENGTH, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "type", prop);
    CHECK_STATUS;

    options = argv[1];
    status = napi_get_named_property(env, options, "serverID", &prop);
    CHECK_STATUS;
    status = napi_get_value_int32(env, prop, &serverID);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "serverID", prop);
    CHECK_STATUS;

    status = napi_get_named_property(env, options, "portName", &prop);
    CHECK_STATUS;
    status = napi_get_value_string_utf8(env, prop, nullptr, 0, &portNameLen);
    CHECK_STATUS;
    portName = (char*) malloc((portNameLen + 1) * sizeof(char));
    status = napi_get_value_string_utf8(env, prop, portName, portNameLen + 1, &portNameLen);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "portName", prop);
    CHECK_STATUS;

    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::wstring wportName = utf8_conv.from_bytes(portName);

    // Prevent accidental creation of extra port
    portNames = server->getPortNames();
    bool foundPort = false;
    for ( int x = 0 ; x < portNames->length() ; x++ ) {
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x])) {
        foundPort = true;
        break;
      }
    }
    free(portName);
    if (!foundPort) {
      NAPI_THROW_ORB_DESTROY("Cannot retrieve status for an unknown port name.");
    }

    Quentin::Port_ptr port = server->getPort(wportName.data(), 0);

    Quentin::PortListener::PlayPortStatus* gps = &port->getStatus().playStatus();

    // TODO timecodes

    status = napi_create_int32(env, gps->portNumber, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "portID", prop);
    CHECK_STATUS;

    status = napi_create_double(env, gps->speed, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "speed", prop);
    CHECK_STATUS;

    status = napi_create_int64(env, gps->offset, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "offset", prop);
    CHECK_STATUS;

    switch (gps->flags & 0x0f) {
      case 1:
        status = napi_create_string_utf8(env, "readyToPlay", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
      case 2:
        status = napi_create_string_utf8(env, "playing", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
      case 4:
        status = napi_create_string_utf8(env, "jumpReady", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
      case 8:
        status = napi_create_string_utf8(env, "fading", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
      default:
        status = napi_create_string_utf8(env, "unknown", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
    }
    status = napi_set_named_property(env, result, "status", prop);
    CHECK_STATUS;

    status = napi_create_int64(env, gps->endOfData, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "endOfData", prop);
    CHECK_STATUS;

    status = napi_create_int64(env, gps->framesUnused, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "framesUnused", prop);
    CHECK_STATUS;
  }
  catch(CORBA::SystemException& ex) {
    NAPI_THROW_CORBA_EXCEPTION(ex);
  }
  catch(CORBA::Exception& ex) {
    NAPI_THROW_CORBA_EXCEPTION(ex);
  }
  catch(omniORB::fatalException& fe) {
    NAPI_THROW_FATAL_EXCEPTION(fe);
  }

  orb->destroy();
  return result;
}

napi_value releasePort(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID;
  char* portName;
  size_t portNameLen;

  try {
    status = retrieveZonePortal(env, info, &orb, &zp);
    CHECK_STATUS;

    size_t argc = 2;
    napi_value argv[2];
    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_STATUS;

    if (argc < 2) {
      NAPI_THROW_ORB_DESTROY("Options object with server ID, port name and channel must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with server ID, port name and channel.");
    }

    options = argv[1];
    status = napi_get_named_property(env, options, "serverID", &prop);
    CHECK_STATUS;
    status = napi_get_value_int32(env, prop, &serverID);
    CHECK_STATUS;

    status = napi_get_named_property(env, options, "portName", &prop);
    CHECK_STATUS;
    status = napi_get_value_string_utf8(env, prop, nullptr, 0, &portNameLen);
    CHECK_STATUS;
    portName = (char*) malloc((portNameLen + 1) * sizeof(char));
    status = napi_get_value_string_utf8(env, prop, portName, portNameLen + 1, &portNameLen);
    CHECK_STATUS;

    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), 0);

    port->reset();
    port->release();
  }
  catch(CORBA::SystemException& ex) {
    NAPI_THROW_CORBA_EXCEPTION(ex);
  }
  catch(CORBA::Exception& ex) {
    NAPI_THROW_CORBA_EXCEPTION(ex);
  }
  catch(omniORB::fatalException& fe) {
    NAPI_THROW_FATAL_EXCEPTION(fe);
  }

  orb->destroy();
  status = napi_get_boolean(env, true, &prop);
  CHECK_STATUS;
  return prop;
}


napi_value Init(napi_env env, napi_value exports) {
  napi_status status;

  napi_property_descriptor desc[] = {
    DECLARE_NAPI_METHOD("testConnection", testConnection),
    DECLARE_NAPI_METHOD("getZoneInfo", getZoneInfo),
    DECLARE_NAPI_METHOD("getServers", getServers),
    DECLARE_NAPI_METHOD("createPlayPort", createPlayPort),
    DECLARE_NAPI_METHOD("getPlayPortStatus", getPlayPortStatus),
    DECLARE_NAPI_METHOD("releasePort", releasePort)
  };
  status = napi_define_properties(env, exports, 6, desc);

  return exports;
}

NAPI_MODULE(sofie_quantel, Init)
