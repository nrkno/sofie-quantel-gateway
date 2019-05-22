#include <Quentin.hh>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwchar>
#include <string>
#include <cstring>
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

#define START 0
#define STOP 1
#define JUMP 2
#define TRANSITION 3

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
    } else {
      status = napi_get_boolean(env, false, &prop);
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

    if (!port->actionAtTrigger(START, Quentin::Port::trActStart)) {
      NAPI_THROW_ORB_DESTROY("Failed to enable start trigger on port.");
    }
    if (!port->actionAtTrigger(STOP, Quentin::Port::trActStop)) {
      NAPI_THROW_ORB_DESTROY("Failed to enable stop trigger on port.");
    }
    /* TODO enable when needed
    if (!port->actionAtTrigger(JUMP, Quentin::Port::trActJump)) {
      NAPI_THROW_ORB_DESTROY("Failed to enable jump trigger on port.");
    };
    if (!port->actionAtTrigger(TRANSITION, Quentin::Port::trActTransition)) {
      NAPI_THROW_ORB_DESTROY("Failed to enable transition trigger on port.");
    }; */
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
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x]) == 0) {
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

    printf("Flags %i\n", gps->flags);
    switch (gps->flags & 0x0f) {
      case 1:
        status = napi_create_string_utf8(env, "readyToPlay", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 2:
        status = napi_create_string_utf8(env, "playing", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 3:
        status = napi_create_string_utf8(env, "playing&readyToPlay", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 4:
        status = napi_create_string_utf8(env, "jumpReady", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 5:
        status = napi_create_string_utf8(env, "jumpReady&readyToPlay", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 6:
        status = napi_create_string_utf8(env, "jumpReady&playing", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 7:
        status = napi_create_string_utf8(env, "jumpReady&readyToPlay&playing", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
      case 8:
        status = napi_create_string_utf8(env, "fading", NAPI_AUTO_LENGTH, &prop);
        CHECK_STATUS;
        break;
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
  napi_value prop, options;
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
    std::wstring wportName = utf8_conv.from_bytes(portName);

    // Prevent accidental creation of extra port
    portNames = server->getPortNames();
    bool foundPort = false;
    for ( int x = 0 ; x < portNames->length() ; x++ ) {
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x]) == 0) {
        foundPort = true;
        break;
      }
    }
    free(portName);
    if (!foundPort) {
      NAPI_THROW_ORB_DESTROY("Cannot release a port with an unknown port name.");
    }

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

napi_value getAllFragments(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, options, frag, fragprop;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t clipID;
  char rushID[33];

  try {
    status = retrieveZonePortal(env, info, &orb, &zp);
    CHECK_STATUS;

    size_t argc = 2;
    napi_value argv[2];
    status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_STATUS;

    if (argc < 2) {
      NAPI_THROW_ORB_DESTROY("Options object with clip ID must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with a clip ID.");
    }

    status = napi_create_object(env, &result);
    CHECK_STATUS;
    status = napi_create_string_utf8(env, "ServerFramgments", NAPI_AUTO_LENGTH, &prop);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "type", prop);
    CHECK_STATUS;

    options = argv[1];
    status = napi_get_named_property(env, options, "clipID", &prop);
    CHECK_STATUS;
    status = napi_get_value_int32(env, prop, &clipID);
    CHECK_STATUS;
    status = napi_set_named_property(env, result, "clipID", prop);
    CHECK_STATUS;

    Quentin::ServerFragments_var fragments = zp->getAllFragments(clipID);

    status = napi_create_array(env, &prop);
    CHECK_STATUS;

    for ( int x = 0 ; x < fragments->length() ; x++ ) {
      status = napi_create_object(env, &frag);
      CHECK_STATUS;

      switch (fragments[x].fragmentData._d()) {
      case Quentin::FragmentType::videoFragment:
        status = napi_create_string_utf8(env, "VideoFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::audioFragment:
        status = napi_create_string_utf8(env, "AudioFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::auxFragment:
        status = napi_create_string_utf8(env, "AUXFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::flagsFragment:
        status = napi_create_string_utf8(env, "FlagsFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::timecodeFragment:
        status = napi_create_string_utf8(env, "TimecodeFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::cropFragment:
        status = napi_create_string_utf8(env, "CropFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::panZoomFragment:
        status = napi_create_string_utf8(env, "PanZoomFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::speedFragment:
        status = napi_create_string_utf8(env, "SpeedFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::multiCamFragment:
        status = napi_create_string_utf8(env, "MultiCamFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::ccFragment:
        status = napi_create_string_utf8(env, "CCFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::effectFragment:
        status = napi_create_string_utf8(env, "EffectFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      default:
        status = napi_create_string_utf8(env, "ServerFragment", NAPI_AUTO_LENGTH, &fragprop);
        CHECK_STATUS;
        break;
      }
      status = napi_set_named_property(env, frag, "type", fragprop);
      CHECK_STATUS;

      status = napi_create_int32(env, fragments[x].trackNum, &fragprop);
      CHECK_STATUS;
      status = napi_set_named_property(env, frag, "trackNum", fragprop);
      CHECK_STATUS;

      status = napi_create_int32(env, fragments[x].start, &fragprop);
      CHECK_STATUS;
      status = napi_set_named_property(env, frag, "start", fragprop);
      CHECK_STATUS;

      status = napi_create_int32(env, fragments[x].finish, &fragprop);
      CHECK_STATUS;
      status = napi_set_named_property(env, frag, "finish", fragprop);
      CHECK_STATUS;

      switch (fragments[x].fragmentData._d()) {
      case Quentin::FragmentType::videoFragment:
        sprintf(rushID, "%016llx%016llx",
          fragments[x].fragmentData.videoFragmentData().rushID.first,
          fragments[x].fragmentData.videoFragmentData().rushID.second);
        status = napi_create_string_utf8(env, rushID, 32, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "rushID", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().format, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "format", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().poolID, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "poolID", fragprop);
        CHECK_STATUS;

        status = napi_create_int64(env, fragments[x].fragmentData.videoFragmentData().poolFrame, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "poolFrame", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().skew, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "skew", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().rushFrame, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "rushFrame", fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::audioFragment:
        sprintf(rushID, "%016llx%016llx",
          fragments[x].fragmentData.audioFragmentData().rushID.first,
          fragments[x].fragmentData.audioFragmentData().rushID.second);
        status = napi_create_string_utf8(env, rushID, 32, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "rushID", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().format, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "format", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().poolID, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "poolID", fragprop);
        CHECK_STATUS;

        status = napi_create_int64(env, fragments[x].fragmentData.audioFragmentData().poolFrame, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "poolFrame", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().skew, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "skew", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().rushFrame, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "rushFrame", fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::auxFragment:
        sprintf(rushID, "%016llx%016llx",
          fragments[x].fragmentData.auxFragmentData().rushID.first,
          fragments[x].fragmentData.auxFragmentData().rushID.second);
        status = napi_create_string_utf8(env, rushID, 32, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "rushID", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().format, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "format", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().poolID, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "poolID", fragprop);
        CHECK_STATUS;

        status = napi_create_int64(env, fragments[x].fragmentData.auxFragmentData().poolFrame, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "poolFrame", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().skew, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "skew", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().rushFrame, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "rushFrame", fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::flagsFragment:
        // TODO
        break;
      case Quentin::FragmentType::timecodeFragment:
        // TODO
        break;
      case Quentin::FragmentType::cropFragment:
        // TODO
        break;
      case Quentin::FragmentType::panZoomFragment:
        // TODO
        break;
      case Quentin::FragmentType::speedFragment:
        // TODO
        break;
      case Quentin::FragmentType::multiCamFragment:
        // TODO
        break;
      case Quentin::FragmentType::ccFragment:
        sprintf(rushID, "%016llx%016llx",
          fragments[x].fragmentData.ccFragmentData().ccID.first,
          fragments[x].fragmentData.ccFragmentData().ccID.second);
        status = napi_create_string_utf8(env, rushID, 32, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "ccID", fragprop);
        CHECK_STATUS;

        status = napi_create_int32(env, fragments[x].fragmentData.ccFragmentData().ccType, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "ccType", fragprop);
        CHECK_STATUS;
        break;
      case Quentin::FragmentType::effectFragment:
        status = napi_create_int32(env, fragments[x].fragmentData.effectFragmentData().effectID, &fragprop);
        CHECK_STATUS;
        status = napi_set_named_property(env, frag, "effectID", fragprop);
        CHECK_STATUS;
        break;
      default:
        break;
      }

      status = napi_set_element(env, prop, x, frag);
      CHECK_STATUS;
    }
    status = napi_set_named_property(env, result, "fragments", prop);
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

napi_value loadPlayPort(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value prop, subprop, options, fragprop;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID;
  int32_t offset = 0;
  char* portName;
  size_t portNameLen;
  Quentin::WStrings_var portNames;
  Quentin::ServerFragments* fragments;
  char rushID[33];
  char typeName[32];
  uint32_t fragmentNo;
  uint32_t fragmentCount = 0;

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

    status = napi_get_named_property(env, options, "offset", &prop);
    CHECK_STATUS;
    status = napi_typeof(env, prop, &type);
    CHECK_STATUS;
    if (type == napi_number) {
      status = napi_get_value_int32(env, prop, &offset);
      CHECK_STATUS;
    }

    status = napi_get_named_property(env, options, "fragments", &prop);
    CHECK_STATUS;
    status = napi_get_named_property(env, prop, "fragments", &fragprop);
    CHECK_STATUS;

    Quentin::ServerFragments fragments;
    status = napi_get_array_length(env, fragprop, &fragmentNo);
    CHECK_STATUS;
    fragments.length(fragmentNo);

    for ( int i = 0 ; i < fragmentNo ; i++ ) {
      status = napi_get_element(env, fragprop, i, &prop);
      CHECK_STATUS;

      Quentin::ServerFragment sf = {};
      Quentin::PositionData vfd = {};
      Quentin::ServerFragmentData sfd;
      std::string rushIDStr;

      status = napi_get_named_property(env, prop, "trackNum", &subprop);
      CHECK_STATUS;
      status = napi_get_value_int32(env, subprop, (int32_t *) &sf.trackNum);
      CHECK_STATUS;

      status = napi_get_named_property(env, prop, "start", &subprop);
      CHECK_STATUS;
      status = napi_get_value_int32(env, subprop, (int32_t *) &sf.start);
      CHECK_STATUS;

      status = napi_get_named_property(env, prop, "finish", &subprop);
      CHECK_STATUS;
      status = napi_get_value_int32(env, subprop, (int32_t *) &sf.finish);
      CHECK_STATUS;

      status = napi_get_named_property(env, prop, "type", &subprop);
      CHECK_STATUS;
      status = napi_get_value_string_utf8(env, subprop, typeName, 32, nullptr);
      CHECK_STATUS;

      printf("Processing fragment of type: %s\n", typeName);

      switch (typeName[0]) {
      case 'V': // VideoFragment
      case 'A': // AudioFragment & AUXFragment

        status = napi_get_named_property(env, prop, "format", &subprop);
        CHECK_STATUS;
        status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.format);
        CHECK_STATUS;

        status = napi_get_named_property(env, prop, "poolID", &subprop);
        CHECK_STATUS;
        status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.poolID);
        CHECK_STATUS;

        status = napi_get_named_property(env, prop, "poolFrame", &subprop);
        CHECK_STATUS;
        status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.poolFrame);
        CHECK_STATUS;

        status = napi_get_named_property(env, prop, "skew", &subprop);
        CHECK_STATUS;
        status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.skew);
        CHECK_STATUS;

        status = napi_get_named_property(env, prop, "rushFrame", &subprop);
        CHECK_STATUS;
        status = napi_get_value_int64(env, subprop, (int64_t *) &vfd.rushFrame);
        CHECK_STATUS;

        status = napi_get_named_property(env, prop, "rushID", &subprop);
        CHECK_STATUS;
        status = napi_get_value_string_utf8(env, subprop, rushID, 33, nullptr);
        CHECK_STATUS;
        rushIDStr.assign(rushID);
        vfd.rushID = {
          (CORBA::LongLong) strtoull(rushIDStr.substr(0, 16).c_str(), nullptr, 16),
          (CORBA::LongLong) strtoull(rushIDStr.substr(16, 32).c_str(), nullptr, 16) };
        vfd.rushFrame = 0;

        if (typeName[0] == 'V') {
          sfd.videoFragmentData(vfd);
        } else if (typeName[1] == 'u') { // AudioFragment
          sfd.audioFragmentData(vfd);
        } else {
          sfd.auxFragmentData(vfd);
        }
        sf.fragmentData = sfd;
        fragments[fragmentCount++] = sf;
        break;
      default:
        break;
      } // switch typeName[0]
    } // for loop through incoming fragments
    fragments.length(fragmentCount);

    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::wstring wportName = utf8_conv.from_bytes(portName);

    // Prevent accidental creation of extra port
    portNames = server->getPortNames();
    bool foundPort = false;
    for ( int x = 0 ; x < portNames->length() ; x++ ) {
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x]) == 0) {
        foundPort = true;
        break;
      }
    }
    free(portName);
    if (!foundPort) {
      NAPI_THROW_ORB_DESTROY("Cannot load a port with an unknown port name.");
    }

    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), 0);
    port->load(offset, fragments);
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
  return nullptr;
}

napi_value trigger(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID, trigger;
  int32_t offset = -1;
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
      NAPI_THROW_ORB_DESTROY("Options object with server ID, port name and action must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with server ID, port name and action.");
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

    status = napi_get_named_property(env, options, "trigger", &prop);
    CHECK_STATUS;
    status = napi_get_value_int32(env, prop, &trigger);
    CHECK_STATUS;

    status = napi_get_named_property(env, options, "offset", &prop);
    CHECK_STATUS;
    status = napi_typeof(env, prop, &type);
    CHECK_STATUS;
    if (type == napi_number) {
      status = napi_get_value_int32(env, prop, &offset);
      CHECK_STATUS;
    }

    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::wstring wportName = utf8_conv.from_bytes(portName);

    // Prevent accidental creation of extra port
    portNames = server->getPortNames();
    bool foundPort = false;
    for ( int x = 0 ; x < portNames->length() ; x++ ) {
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x]) == 0) {
        foundPort = true;
        break;
      }
    }
    free(portName);
    if (!foundPort) {
      NAPI_THROW_ORB_DESTROY("Cannot trigger action on a port with an unknown port name.");
    }

    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), 0);

    if (!port->setTrigger(trigger,
      offset >= 0 ? Quentin::Port::trModeOffset : Quentin::Port::trModeNow,
      offset >= 0 ? offset : 0) ) {
      if (offset >= 0) {
        NAPI_THROW_ORB_DESTROY("Failed to trigger action at offset.");
      } else {
        NAPI_THROW_ORB_DESTROY("Failed to trigger action immediately.");
      }
    };
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

napi_value qJump(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID;
  int32_t offset = 0;
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
      NAPI_THROW_ORB_DESTROY("Options object with server ID, port name and jump offset must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with server ID, port name and jump offset.");
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

    status = napi_get_named_property(env, options, "offset", &prop);
    CHECK_STATUS;
    status = napi_typeof(env, prop, &type);
    CHECK_STATUS;
    if (type == napi_number) {
      status = napi_get_value_int32(env, prop, &offset);
      CHECK_STATUS;
    }

    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::wstring wportName = utf8_conv.from_bytes(portName);

    // Prevent accidental creation of extra port
    portNames = server->getPortNames();
    bool foundPort = false;
    for ( int x = 0 ; x < portNames->length() ; x++ ) {
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x]) == 0) {
        foundPort = true;
        break;
      }
    }
    free(portName);
    if (!foundPort) {
      NAPI_THROW_ORB_DESTROY("Cannot set jump point on a port with an unknown port name.");
    }

    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), 0);

    // Disable preload is documented as not implemented
    port->jump(offset, false);
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

napi_value setJump(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID;
  int32_t offset = 0;
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
      NAPI_THROW_ORB_DESTROY("Options object with server ID, port name and jump offset must be provided.");
    }
    status = napi_typeof(env, argv[1], &type);
    CHECK_STATUS;
    status = napi_is_array(env, argv[1], &isArray);
    CHECK_STATUS;
    if (isArray || type != napi_object) {
      NAPI_THROW_ORB_DESTROY("Argument must be an options object with server ID, port name and jump offset.");
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

    status = napi_get_named_property(env, options, "offset", &prop);
    CHECK_STATUS;
    status = napi_typeof(env, prop, &type);
    CHECK_STATUS;
    if (type == napi_number) {
      status = napi_get_value_int32(env, prop, &offset);
      CHECK_STATUS;
    }

    Quentin::Server_ptr server = zp->getServer(serverID);

    std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    std::wstring wportName = utf8_conv.from_bytes(portName);

    // Prevent accidental creation of extra port
    portNames = server->getPortNames();
    bool foundPort = false;
    for ( int x = 0 ; x < portNames->length() ; x++ ) {
      if (wcscmp(wportName.data(), (const wchar_t *) portNames[x]) == 0) {
        foundPort = true;
        break;
      }
    }
    free(portName);
    if (!foundPort) {
      NAPI_THROW_ORB_DESTROY("Cannot set jump point on a port with an unknown port name.");
    }

    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), 0);

    port->setJump(offset);
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

napi_value getThumbnailSize(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value prop, result;
	napi_valuetype type;
	bool isArray;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	CORBA::Long width(0);
	CORBA::Long height(0);

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;

		zp->getThumbnailSize(0, width, height);

		status = napi_create_object(env, &result);
		CHECK_STATUS;

		status = napi_create_int32(env, width, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "width", prop);
		CHECK_STATUS;

		status = napi_create_int32(env, height, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "height", prop);
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

class ThumbyListener : public POA_Quentin::ThumbnailListener {
public:
	inline ThumbyListener(CORBA::ORB_var orby /*, int32_t tnCount */) : ident(ID++) {
		orb = CORBA::ORB::_duplicate(orby);
		// count = tnCount;
	}
	virtual ~ThumbyListener() { free(lastFrame); }
	virtual void newThumbnail(CORBA::Long ident, CORBA::Long offset, CORBA::Long width, CORBA::Long height, const Quentin::Longs & data);
	virtual void noThumbnail(Quentin::ThumbnailListener::NoThumbnailReason reason, CORBA::Long ident, CORBA::Long offset, CORBA::Boolean tryAgainLater, const CORBA::WChar * reasonStr);
  virtual void finished(CORBA::Long ident);
	virtual char* getLastFrame();
	virtual size_t getLastFrameSize();
	virtual inline int32_t getIdent() { return ident; };
private:
	CORBA::ORB_ptr orb;
	size_t lastFrameSize = 0;
	char* lastFrame = nullptr;
	static int32_t ID;
	int ident;
};

int32_t ThumbyListener::ID = 0;

void ThumbyListener::newThumbnail(CORBA::Long ident, CORBA::Long offset, CORBA::Long width, CORBA::Long height, const Quentin::Longs & data) {
	printf("newThumbnail called - ident %i, offset %i, dimensions %ix%i, data length %i\n", ident, offset, width, height, data.length());
	if (lastFrame != nullptr) free(lastFrame);
	lastFrameSize = data.length() * 4;
	lastFrame = (char*) malloc(lastFrameSize * sizeof(char));
	for ( int x = 0 ; x < data.length() ; x++ ) {
		lastFrame[x * 4] = (data[x] >> 24) & 0xff;
		lastFrame[x * 4 + 1] = (data[x] >> 16) & 0xff;
		lastFrame[x * 4 + 2] = (data[x] >> 8) & 0xff;
		lastFrame[x * 4 + 3] = data[x] & 0xff;
	}
}

void ThumbyListener::noThumbnail(Quentin::ThumbnailListener::NoThumbnailReason reason, CORBA::Long ident, CORBA::Long offset, CORBA::Boolean tryAgainLater, const CORBA::WChar * reasonStr) {
	printf("noThumbnail called\n");
}

void ThumbyListener::finished(CORBA::Long ident) {
	printf("finished called\n");
	orb->shutdown(false);
	CORBA::release(orb);
}

char* ThumbyListener::getLastFrame() {
	return lastFrame;
}

size_t ThumbyListener::getLastFrameSize() {
	return lastFrameSize;
}

napi_value requestThumbnails(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value prop, result;
	napi_valuetype type;
	bool isArray;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::ServerFragments_var fragments;
	int32_t clipID, offset, stride, tnCount;

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;

		size_t argc = 2;
		napi_value argv[2];
		status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
		CHECK_STATUS;

		if (argc < 2) {
			NAPI_THROW_ORB_DESTROY("Options object with clip ID, offset, stride and thumbnail count must be provided.");
		}
		status = napi_typeof(env, argv[1], &type);
		CHECK_STATUS;
		status = napi_is_array(env, argv[1], &isArray);
		CHECK_STATUS;
		if (isArray || type != napi_object) {
			NAPI_THROW_ORB_DESTROY("Argument must be an options object with clip ID, offset, stride and thumbnail count.");
		}

		status = napi_get_named_property(env, argv[1], "clipID", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &clipID);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "offset", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &offset);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "stride", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &stride);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "count", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &tnCount);
		CHECK_STATUS;

		fragments = zp->getTypeFragments(clipID, 0);

		CORBA::Object_var       obj = orb->resolve_initial_references("RootPOA");
		PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

		PortableServer::Servant_var<ThumbyListener> mythumby = new ThumbyListener(orb);
		PortableServer::ObjectId_var mythumbyid = poa->activate_object(mythumby);

		PortableServer::POAManager_var pman = poa->the_POAManager();
		pman->activate();

		Quentin::ThumbnailListener_ptr qtip = Quentin::ThumbnailListener::_duplicate(mythumby->_this());
		zp->requestThumbnails(0, fragments[0].fragmentData.videoFragmentData(), offset, stride, tnCount, mythumby->getIdent(), qtip);

		orb->run();
		CORBA::release(qtip);
		// poa->deactivate_object(mythumbyid);
		// pman->deactivate(true, true);
		void* resultData;

		status = napi_create_buffer_copy(env, mythumby->getLastFrameSize(), mythumby->getLastFrame(),
		  &resultData, &result);
		CHECK_STATUS;
		printf("End of try scope.\n");
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
	printf("Calling orb->destroy()\n");
	orb->destroy();
	printf("Reached return statement.\n");
	return result;
}

napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value start, stop, jump, transition;
  status = napi_create_int32(env, START, &start);
  status = napi_create_int32(env, STOP, &stop);
  status = napi_create_int32(env, JUMP, &jump);
  status = napi_create_int32(env, TRANSITION, &transition);

  napi_property_descriptor desc[] = {
    DECLARE_NAPI_METHOD("testConnection", testConnection),
    DECLARE_NAPI_METHOD("getZoneInfo", getZoneInfo),
    DECLARE_NAPI_METHOD("getServers", getServers),
    DECLARE_NAPI_METHOD("createPlayPort", createPlayPort),
    DECLARE_NAPI_METHOD("getPlayPortStatus", getPlayPortStatus),
    DECLARE_NAPI_METHOD("releasePort", releasePort),
    DECLARE_NAPI_METHOD("getAllFragments", getAllFragments),
    DECLARE_NAPI_METHOD("loadPlayPort", loadPlayPort),
    DECLARE_NAPI_METHOD("trigger", trigger),
    DECLARE_NAPI_METHOD("jump", qJump),
    DECLARE_NAPI_METHOD("setJump", setJump),
		DECLARE_NAPI_METHOD("getThumbnailSize", getThumbnailSize),
		DECLARE_NAPI_METHOD("requestThumbnails", requestThumbnails),
    { "START", nullptr, nullptr, nullptr, nullptr, start, napi_enumerable, nullptr },
    { "STOP", nullptr, nullptr, nullptr, nullptr, stop, napi_enumerable, nullptr },
    { "JUMP", nullptr, nullptr, nullptr, nullptr, jump, napi_enumerable, nullptr },
    { "TRANSITION", nullptr, nullptr, nullptr, nullptr, transition, napi_enumerable, nullptr },
  };
  status = napi_define_properties(env, exports, 17, desc);

  return exports;
}

NAPI_MODULE(sofie_quantel, Init)
