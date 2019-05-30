#include "port.h"

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
    if (!port->actionAtTrigger(JUMP, Quentin::Port::trActJump)) {
      NAPI_THROW_ORB_DESTROY("Failed to enable jump trigger on port.");
    };
		/* TODO enable when needed
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
