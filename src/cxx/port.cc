#include "port.h"

int32_t portCounter = 1;

void createPlayPortExecute(napi_env env, void* data) {
	createPlayPortCarrier* c = (createPlayPortCarrier*) data;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	int32_t portID = portCounter++;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);

		Quentin::Server_var server = zp->getServer(c->serverID);

		if (c->portID >= 0) {
			portID = c->portID;
		}

		Quentin::Port_var port = server->getPort(utf8_conv.from_bytes(c->portName).data(), portID);
		if (port == nullptr) {
			orb->destroy();
			c->status = QGW_GET_PORT_FAIL;
			c->errorMsg = "Unable to create port. It is likely that all ports are assigned.";
			return;
		}
		c->portID = portID;
		c->assigned = port->assignChannel(c->channelNo, c->audioOnly ? 1 : 0);

		CORBA::Boolean playing = port->setMode(Quentin::Port::PortMode::playing);
		if (!playing) {
			orb->destroy();
			c->status = QGW_SET_MODE_FAIL;
			c->errorMsg = "Failed to set play mode for new port.";
			return;
		}

		if (!port->actionAtTrigger(START, Quentin::Port::trActStart)) {
			orb->destroy();
			c->status = QGW_TRIGGER_SETUP_FAIL;
			c->errorMsg = "Unable set set up START trigger action for new port.";
			return;
		}
		if (!port->actionAtTrigger(STOP, Quentin::Port::trActStop)) {
			orb->destroy();
			c->status = QGW_TRIGGER_SETUP_FAIL;
			c->errorMsg = "Unable set set up STOP trigger action for new port.";
			return;
		}
		if (!port->actionAtTrigger(JUMP, Quentin::Port::trActJump)) {
			orb->destroy();
			c->status = QGW_TRIGGER_SETUP_FAIL;
			c->errorMsg = "Unable set set up JUMP trigger action for new port.";
			return;
		};
		/* TODO enable when needed
		if (!port->actionAtTrigger(TRANSITION, Quentin::Port::trActTransition)) {
			NAPI_THROW_ORB_DESTROY("Failed to enable transition trigger on port.");
		}; */
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

void createPlayPortComplete(napi_env env, napi_status asyncStatus, void* data) {
	createPlayPortCarrier* c = (createPlayPortCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Test connection failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;
	c->status = napi_create_string_utf8(env, "PortInfo", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->serverID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "serverID", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->channelNo, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "channelNo", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->audioOnly, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "audioOnly", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portName.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portName", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->portID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portID", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->assigned, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "assigned", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value createPlayPort(napi_env env, napi_callback_info info) {
  createPlayPortCarrier* c = new createPlayPortCarrier;
  napi_value promise, prop, options, resourceName;
  napi_valuetype type;
  bool isArray;
  char* portName;
  size_t portNameLen;
  bool audioOnly;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

	printf("Entering create play port\n");

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
    REJECT_ERROR_RETURN("Options object with server ID, port name and channel must be provided.",
	    QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with server ID, port name and channel.",
	    QGW_INVALID_ARGS);
  }

  options = argv[1];
  c->status = napi_get_named_property(env, options, "serverID", &prop);
  REJECT_RETURN;
  c->status = napi_get_value_int32(env, prop, &c->serverID);
  REJECT_RETURN;

  c->status = napi_get_named_property(env, options, "channelNo", &prop);
  REJECT_RETURN;
  c->status = napi_get_value_int32(env, prop, &c->channelNo);
  REJECT_RETURN;

  c->status = napi_has_named_property(env, options, "audioOnly", &audioOnly);
  REJECT_RETURN;
  if (audioOnly) {
    c->status = napi_get_named_property(env, options, "audioOnly", &prop);
    REJECT_RETURN;
    c->status = napi_get_value_bool(env, prop, &c->audioOnly);
    REJECT_RETURN;
  } else {
    c->audioOnly = false;
  }

  c->status = napi_get_named_property(env, options, "portName", &prop);
	REJECT_RETURN;
  c->status = napi_get_value_string_utf8(env, prop, nullptr, 0, &portNameLen);
  REJECT_RETURN;
  portName = (char*) malloc((portNameLen + 1) * sizeof(char));
  c->status = napi_get_value_string_utf8(env, prop, portName, portNameLen + 1, &portNameLen);
  REJECT_RETURN;
	c->portName = std::string(portName);
	free(portName);

	c->status = napi_has_named_property(env, options, "portID", &isArray);
	REJECT_RETURN;
	if (isArray) {
		c->status = napi_get_named_property(env, options, "portID", &prop);
		REJECT_RETURN;
		c->status = napi_get_value_int32(env, prop, &c->portID);
		REJECT_RETURN;
	}

	c->status = napi_create_string_utf8(env, "createPlayPort", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, createPlayPortExecute,
		createPlayPortComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

  return promise;
}

napi_value getPlayPortStatus(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, options, chanList;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t serverID;
  char* portName;
  size_t portNameLen;
  Quentin::WStrings_var portNames;
	Quentin::Longs_var channels;

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
    /* portNames = server->getPortNames();
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
    } */

    Quentin::Port_ptr port = server->getPort(wportName.data(), 0);

    Quentin::PortListener::PlayPortStatus* gps = &port->getStatus().playStatus();

    status = napi_create_string_utf8(env, formatTimecode(gps->refTime), NAPI_AUTO_LENGTH, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "refTime", prop);
		CHECK_STATUS;

		status = napi_create_string_utf8(env, formatTimecode(gps->portTime), NAPI_AUTO_LENGTH, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "portTime", prop);
		CHECK_STATUS;

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

		status = napi_create_string_utf8(env, formatTimecode(gps->outputTime), NAPI_AUTO_LENGTH, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "outputTime", prop);
		CHECK_STATUS;

		channels = port->getChannels();
		status = napi_create_array(env, &chanList);
		CHECK_STATUS;

		for ( uint32_t x = 0 ; x < channels->length() ; x++ ) {
			status = napi_create_int32(env, channels[x], &prop);
			CHECK_STATUS;
			status = napi_set_element(env, chanList, x, prop);
			CHECK_STATUS;
		}

		status = napi_set_named_property(env, result, "channels", chanList);
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
    /* portNames = server->getPortNames();
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
    } */

    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(portName).data(), 0);

		port->setMode(Quentin::Port::PortMode::idle);
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

		status = napi_typeof(env, fragprop, &type);
		CHECK_STATUS;
		if (type == napi_undefined) {
			fragprop = prop;
		}

    Quentin::ServerFragments fragments;
    status = napi_get_array_length(env, fragprop, &fragmentNo);
    CHECK_STATUS;
    fragments.length(fragmentNo);

    for ( uint32_t i = 0 ; i < fragmentNo ; i++ ) {
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
      NAPI_THROW_ORB_DESTROY("Cannot load a port with an unknown port name.");
    } */

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
