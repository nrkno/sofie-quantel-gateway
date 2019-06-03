#include "control.h"

void triggerExecute(napi_env env, void* data) {
	triggerCarrier* c = (triggerCarrier*) data;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);

    Quentin::Server_ptr server = zp->getServer(c->serverID);
    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

    if (!port->setTrigger(c->trigger,
      	c->offset >= 0 ? Quentin::Port::trModeOffset : Quentin::Port::trModeNow,
      	c->offset >= 0 ? c->offset : 0) ) {
      if (c->offset >= 0) {
				orb->destroy();
				c->status = QGW_TRIGGER_OFFSET_FAIL;
        c->errorMsg = "Failed to trigger action at offset.";
				return;
      } else {
				orb->destroy();
				c->status = QGW_TRIGGER_NOW_FAIL;
        c->errorMsg = "Failed to trigger action immediately.";
				return;
      }
    };
	}
	catch(CORBA::SystemException& ex) {
		NAPI_REJECT_SYSTEM_EXCEPTION(ex);
	}
	catch(CORBA::Exception& ex) {
		NAPI_REJECT_CORBA_EXCEPTION(ex);
	}
	catch(omniORB::fatalException& fe) {
		NAPI_REJECT_FATAL_EXCEPTION(fe);
	}

	orb->destroy();
}

void triggerComplete(napi_env env, napi_status asyncStatus, void* data) {
	triggerCarrier* c = (triggerCarrier*) data;
	napi_value result;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Test connection failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_get_boolean(env, true, &result);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value trigger(napi_env env, napi_callback_info info) {
	triggerCarrier* c = new triggerCarrier;
  napi_value promise, prop, options, resourceName;
  napi_valuetype type;
  bool isArray;
  char* portName;
  size_t portNameLen;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

  c->status = napi_create_promise(env, &c->_deferred, &promise);
  REJECT_RETURN;

  size_t argc = 2;
  napi_value argv[2];
  c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Connection test must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

  if (argc < 2) {
    REJECT_ERROR_RETURN("Options object with server ID, port name and trigger action must be provided.",
			QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with server ID, port name and trigger action.",
			QGW_INVALID_ARGS);
  }

  options = argv[1];
  c->status = napi_get_named_property(env, options, "serverID", &prop);
  REJECT_RETURN;
  c->status = napi_get_value_int32(env, prop, &c->serverID);
  REJECT_RETURN;

  c->status = napi_get_named_property(env, options, "portName", &prop);
  REJECT_RETURN;
  c->status = napi_get_value_string_utf8(env, prop, nullptr, 0, &portNameLen);
  REJECT_RETURN;
  portName = (char*) malloc((portNameLen + 1) * sizeof(char));
  c->status = napi_get_value_string_utf8(env, prop, portName, portNameLen + 1, &portNameLen);
  REJECT_RETURN;
	c->portName = std::string(portName);
	free(portName);

  c->status = napi_get_named_property(env, options, "trigger", &prop);
	REJECT_RETURN;
  c->status = napi_get_value_int32(env, prop, &c->trigger);
	REJECT_RETURN;

  c->status = napi_get_named_property(env, options, "offset", &prop);
  REJECT_RETURN;
  c->status = napi_typeof(env, prop, &type);
  REJECT_RETURN;
  if (type == napi_number) {
    c->status = napi_get_value_int32(env, prop, &c->offset);
    REJECT_RETURN;
  }

	c->status = napi_create_string_utf8(env, "Trigger", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, triggerExecute,
		triggerComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

  return promise;
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
