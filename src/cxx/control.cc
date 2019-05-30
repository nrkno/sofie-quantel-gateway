#include "control.h"

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
    /* std::wstring wportName = utf8_conv.from_bytes(portName);

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
    } */

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
    /* std::wstring wportName = utf8_conv.from_bytes(portName);

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
    } */

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
    /* std::wstring wportName = utf8_conv.from_bytes(portName);

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
    } */

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
