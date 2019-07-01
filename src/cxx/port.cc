/*
	 Copyright (c) 2019 Norsk rikskringkasting AS (NRK)

	 This file is part of Sofie: The Modern TV News Studio Automation
	 System (Quantel gateway)

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License along
	 with this program; if not, write to the Free Software Foundation, Inc.,
	 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "port.h"

int32_t portCounter = 1;

void createPlayPortExecute(napi_env env, void* data) {
	createPlayPortCarrier* c = (createPlayPortCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	int32_t portID = portCounter++;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		Quentin::Server_var server = zp->getServer(c->serverID);

		if (c->portID >= 0) {
			portID = c->portID;
		}

		Quentin::Port_var port = server->getPort(utf8_conv.from_bytes(c->portName).data(), portID);
		if (port == nullptr) {
			c->status = QGW_GET_PORT_FAIL;
			c->errorMsg = "Unable to create port. It is likely that all ports are assigned.";
			return;
		}
		c->portID = portID;
		c->assigned = port->assignChannel(c->channelNo, c->audioOnly ? 1 : 0);

		CORBA::Boolean playing = port->setMode(Quentin::Port::PortMode::playing);
		if (!playing) {
			c->status = QGW_SET_MODE_FAIL;
			c->errorMsg = "Failed to set play mode for new port.";
			return;
		}

		if (!port->actionAtTrigger(START, Quentin::Port::trActStart)) {
			c->status = QGW_TRIGGER_SETUP_FAIL;
			c->errorMsg = "Unable set set up START trigger action for new port.";
			return;
		}
		if (!port->actionAtTrigger(STOP, Quentin::Port::trActStop)) {
			c->status = QGW_TRIGGER_SETUP_FAIL;
			c->errorMsg = "Unable set set up STOP trigger action for new port.";
			return;
		}
		if (!port->actionAtTrigger(JUMP, Quentin::Port::trActJump)) {
			c->status = QGW_TRIGGER_SETUP_FAIL;
			c->errorMsg = "Unable set set up JUMP trigger action for new port.";
			return;
		};
		/* TODO enable when or if needed
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
}

void createPlayPortComplete(napi_env env, napi_status asyncStatus, void* data) {
	createPlayPortCarrier* c = (createPlayPortCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Create play port failed to complete.";
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
		REJECT_ERROR_RETURN("Create play port must be provided with a IOR reference to an ISA server.",
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

void getPlayPortExecute(napi_env env, void* data) {
	playPortStatusCarrier* c = (playPortStatusCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::Longs_var channels;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

	 	Quentin::Server_ptr server = zp->getServer(c->serverID);

	 	std::wstring wportName = utf8_conv.from_bytes(c->portName);
		Quentin::Port_var port = server->getPort(wportName.data(), 0);

	 	Quentin::PortListener::PlayPortStatus_var gps;
		gps = port->getStatus().playStatus();

		c->refTime = formatTimecode(gps->refTime);
		c->portTime = formatTimecode(gps->portTime);
		c->portNumber = gps->portNumber;
		c->speed = gps->speed;
		c->offset = gps->offset;
		switch (gps->flags & 0x0f) {
		case 1:
			c->statusFlags = std::string("readyToPlay"); break;
		case 2:
			c->statusFlags = std::string("playing"); break;
		case 3:
			c->statusFlags = std::string("playing&readyToPlay"); break;
		case 4:
			c->statusFlags = std::string("jumpReady"); break;
		case 5:
			c->statusFlags = std::string("jumpReady&readyToPlay"); break;
		case 6:
			c->statusFlags = std::string("jumpReady&playing"); break;
		case 7:
			c->statusFlags = std::string("jumpReady&readyToPlay&playing"); break;
		case 8:
			c->statusFlags = std::string("fading"); break;
		default:
			c->statusFlags = std::string("unknown"); break;
	 	}
	 	c->endOfData = gps->endOfData;
	 	c->framesUnused = gps->framesUnused;
	 	c->outputTime = formatTimecode(gps->outputTime);

	 	channels = port->getChannels();
	 	for ( uint32_t x = 0 ; x < channels->length() ; x++ ) {
		 	c->channels.push_back(channels[x]);
	 	}

	 	Quentin::ConfigDescriptionList_var cdl = port->getConfigurations(c->channels.at(0),
		  Quentin::FragmentType::videoFragment, true);
		try {
			Quentin::Longs_var currents = port->getCurrentConfigurations(c->channels.at(0));
			for ( uint32_t x = 0 ; x < currents->length() ; x++ ) {
				for ( uint32_t y = 0 ; y < cdl->length() ; y++ ) {
					if (cdl[y].configNumber == currents[x]) {
						c->videoFormat = utf8_conv.to_bytes(std::wstring(cdl[x].description));
						break;
					}
				}
			}
		}
		catch (CORBA::SystemException& ex) {
			if (std::string(ex._name()) == "UNKNOWN") {
				c->videoFormat = std::string("not available - dummy server");
			}
		}
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
}

void getPlayPortComplete(napi_env env, napi_status asyncStatus, void* data) {
	playPortStatusCarrier* c = (playPortStatusCarrier*) data;
	napi_value result, prop, chanList;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Get play port failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;
	c->status = napi_create_string_utf8(env, "PortStatus", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->serverID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "serverID", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portName.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portName", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->refTime.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "refTime", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portTime.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portTime", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->portNumber, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portID", prop);
	REJECT_STATUS;

	c->status = napi_create_double(env, c->speed, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "speed", prop);
	REJECT_STATUS;

	c->status = napi_create_int64(env, c->offset, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "offset", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->statusFlags.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "status", prop);
	REJECT_STATUS;

	c->status = napi_create_int64(env, c->endOfData, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "endOfData", prop);
	REJECT_STATUS;

	c->status = napi_create_int64(env, c->framesUnused, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "framesUnused", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->outputTime.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "outputTime", prop);
	REJECT_STATUS;

	c->status = napi_create_array(env, &chanList);
	REJECT_STATUS;

	for ( uint32_t x = 0 ; x < c->channels.size() ; x++ ) {
		c->status = napi_create_int32(env, c->channels.at(x), &prop);
		REJECT_STATUS;
		c->status = napi_set_element(env, chanList, x, prop);
		REJECT_STATUS;
	}
	c->status = napi_set_named_property(env, result, "channels", chanList);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->videoFormat.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "videoFormat", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getPlayPortStatus(napi_env env, napi_callback_info info) {
  playPortStatusCarrier* c = new playPortStatusCarrier;
  napi_value promise, resourceName, options, prop;
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
		REJECT_ERROR_RETURN("Play port status must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with server ID and port name must be provided.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with server ID and port name.",
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

	c->status = napi_create_string_utf8(env, "GetPlayPortStatus", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, getPlayPortExecute,
    getPlayPortComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void releasePortExecute(napi_env env, void* data) {
	releasePortCarrier* c = (releasePortCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		Quentin::Server_ptr server = zp->getServer(c->serverID);
    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

		port->setMode(Quentin::Port::PortMode::idle);
    port->reset();
    port->release();
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
}

void releasePortComplete(napi_env env, napi_status asyncStatus, void* data) {
	releasePortCarrier* c = (releasePortCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Release port failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "ReleaseStatus", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->serverID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "serverID", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portName.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portName", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, true, &prop);
  REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "released", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value releasePort(napi_env env, napi_callback_info info) {
	releasePortCarrier* c = new releasePortCarrier;
	napi_value promise, resourceName, options, prop;
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
		REJECT_ERROR_RETURN("Release port must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with server ID and port name must be provided.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with server ID and port name.",
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

	c->status = napi_create_string_utf8(env, "ReleasePort", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, releasePortExecute,
    releasePortComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void wipeExecute(napi_env env, void* data) {
	wipeCarrier* c = (wipeCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		Quentin::Server_ptr server = zp->getServer(c->serverID);
    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

		Quentin::PortListener::PlayPortStatus_var gps;
		gps = port->getStatus().playStatus();

		if (c->frames == 0x7fffffff) {
			c->frames = gps->offset - c->start;
			c->frames = (c->frames < 0) ? 0 : c->frames;
		}

		c->wiped = port->wipe(c->start, c->frames);
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
}

void wipeComplete(napi_env env, napi_status asyncStatus, void* data) {
	wipeCarrier* c = (wipeCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Wipe failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "WipeResult", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->serverID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "serverID", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portName.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portName", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->start, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "start", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->frames, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "frames", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->wiped, &prop);
  REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "wiped", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value wipe(napi_env env, napi_callback_info info) {
	wipeCarrier* c = new wipeCarrier;
	napi_value promise, resourceName, options, prop;
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
		REJECT_ERROR_RETURN("Wipe must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with server ID and port name must be provided.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with server ID and port name.",
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

	c->status = napi_get_named_property(env, options, "start", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_number) {
		c->status = napi_get_value_int32(env, prop, &c->start);
		REJECT_RETURN;
	}

	c->status = napi_get_named_property(env, options, "frames", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_number) {
		c->status = napi_get_value_int32(env, prop, &c->frames);
		REJECT_RETURN;
	}

	c->status = napi_create_string_utf8(env, "Wipe", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, wipeExecute,
    wipeComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void loadPlayPortExecute(napi_env env, void* data) {
	loadPlayPortCarrier* c = (loadPlayPortCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

	  Quentin::Server_ptr server = zp->getServer(c->serverID);

	  Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);
	  port->load(c->offset, c->fragments);
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
}

void loadPlayPortComplete(napi_env env, napi_status asyncStatus, void* data) {
	loadPlayPortCarrier* c = (loadPlayPortCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Load play port failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "PortLoadStatus", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->serverID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "serverID", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portName.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portName", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->fragments.length(), &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "fragmentCount", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->offset, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "offset", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value loadPlayPort(napi_env env, napi_callback_info info) {
  loadPlayPortCarrier* c = new loadPlayPortCarrier;
  napi_value promise, prop, subprop, options, fragprop, resourceName;
  napi_valuetype type;
  bool isArray;

  char* portName;
  size_t portNameLen;
  char rushID[33];
	char timecode[12];
  char typeName[32];
  uint32_t fragmentNo;
  uint32_t fragmentCount = 0;
	char* isaIOR = nullptr;
	size_t iorLen = 0;
	char* noteStr = nullptr;
	size_t noteLen = 0;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

  c->status = napi_create_promise(env, &c->_deferred, &promise);
  REJECT_RETURN;

  size_t argc = 2;
  napi_value argv[2];
  c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Load plat port must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

  if (argc < 2) {
    REJECT_ERROR_RETURN("Options object with server ID, port name and fragments must be provided.",
		 	QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with server ID, port name and fragments.",
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

  c->status = napi_get_named_property(env, options, "offset", &prop);
  REJECT_RETURN;
  c->status = napi_typeof(env, prop, &type);
  REJECT_RETURN;
  if (type == napi_number) {
    c->status = napi_get_value_int32(env, prop, &c->offset);
    REJECT_RETURN;
  }

  c->status = napi_get_named_property(env, options, "fragments", &prop);
  REJECT_RETURN;
  c->status = napi_get_named_property(env, prop, "fragments", &fragprop);
  REJECT_RETURN;

	c->status = napi_typeof(env, fragprop, &type);
	REJECT_RETURN;
	if (type == napi_undefined) {
		fragprop = prop;
	}

  c->status = napi_get_array_length(env, fragprop, &fragmentNo);
  REJECT_RETURN;
	printf("Number of fragments is %i\n", fragmentNo);
  c->fragments.length(fragmentNo);

  for ( uint32_t i = 0 ; i < fragmentNo ; i++ ) {
    c->status = napi_get_element(env, fragprop, i, &prop);
    REJECT_RETURN;

    Quentin::ServerFragment sf = {};
    Quentin::PositionData vfd = {};
		Quentin::ServerTimecodeFragment stf = {};
		Quentin::ServerAspectFragment saf = {};
		Quentin::ServerCropFragment scf = {};
		Quentin::ServerCCFragment sccf = {};
		Quentin::ServerSpeedFragment spdf = {};
		Quentin::ServerEffectFragment seff = {};
		Quentin::ServerMultiCamFragment smcf = {};
		Quentin::ServerPanZoomFragment spzf = {};
		Quentin::ServerFlagsFragment sff = {};
		Quentin::ServerNoteFragment sntf = {};
    Quentin::ServerFragmentData sfd;
    std::string rushIDStr, tcStr;

    c->status = napi_get_named_property(env, prop, "trackNum", &subprop);
    REJECT_RETURN;
    c->status = napi_get_value_int32(env, subprop, (int32_t *) &sf.trackNum);
    REJECT_RETURN;

    c->status = napi_get_named_property(env, prop, "start", &subprop);
    REJECT_RETURN;
    c->status = napi_get_value_int32(env, subprop, (int32_t *) &sf.start);
    REJECT_RETURN;

    c->status = napi_get_named_property(env, prop, "finish", &subprop);
    REJECT_RETURN;
    c->status = napi_get_value_int32(env, subprop, (int32_t *) &sf.finish);
    REJECT_RETURN;

    c->status = napi_get_named_property(env, prop, "type", &subprop);
    REJECT_RETURN;
    c->status = napi_get_value_string_utf8(env, subprop, typeName, 32, nullptr);
    REJECT_RETURN;

    switch (typeName[0]) {
    case 'V': // VideoFragment
    case 'A': // AudioFragment & AUXFragment & Aspect Fragment
			if (typeName[1] == 's') {
				c->status = napi_get_named_property(env, prop, "width", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &saf.width);
				REJECT_RETURN;

				c->status = napi_get_named_property(env, prop, "height", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &saf.height);
				REJECT_RETURN;

				sfd.aspectFragmentData(saf);
				sf.fragmentData = sfd;
				c->fragments[fragmentCount++] = sf;
				break;
			}
      c->status = napi_get_named_property(env, prop, "format", &subprop);
      REJECT_RETURN;
      c->status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.format);
      REJECT_RETURN;

      c->status = napi_get_named_property(env, prop, "poolID", &subprop);
      REJECT_RETURN;
      c->status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.poolID);
      REJECT_RETURN;

      c->status = napi_get_named_property(env, prop, "poolFrame", &subprop);
      REJECT_RETURN;
      c->status = napi_get_value_int64(env, subprop, (int64_t *) &vfd.poolFrame);
      REJECT_RETURN;

      c->status = napi_get_named_property(env, prop, "skew", &subprop);
      REJECT_RETURN;
      c->status = napi_get_value_int32(env, subprop, (int32_t *) &vfd.skew);
      REJECT_RETURN;

      c->status = napi_get_named_property(env, prop, "rushFrame", &subprop);
      REJECT_RETURN;
      c->status = napi_get_value_int64(env, subprop, (int64_t *) &vfd.rushFrame);
      REJECT_RETURN;

      c->status = napi_get_named_property(env, prop, "rushID", &subprop);
      REJECT_RETURN;
      c->status = napi_get_value_string_utf8(env, subprop, rushID, 33, nullptr);
      REJECT_RETURN;
      rushIDStr.assign(rushID);
      vfd.rushID = {
        (CORBA::LongLong) strtoull(rushIDStr.substr(0, 16).c_str(), nullptr, 16),
        (CORBA::LongLong) strtoull(rushIDStr.substr(16, 32).c_str(), nullptr, 16) };

      if (typeName[0] == 'V') {
        sfd.videoFragmentData(vfd);
      } else if (typeName[1] == 'u') { // AudioFragment
        sfd.audioFragmentData(vfd);
      } else {
        sfd.auxFragmentData(vfd);
      }
      sf.fragmentData = sfd;
      c->fragments[fragmentCount++] = sf;
      break;
		case 'T': // TimecodeFragment
			c->status = napi_get_named_property(env, prop, "startTimecode", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_string_utf8(env, subprop, timecode, 12, nullptr);
			REJECT_RETURN;
			tcStr.assign(timecode);
			stf.startTimecode = timecodeFromString(tcStr);

			c->status = napi_get_named_property(env, prop, "userBits", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t*) &stf.userBits);
			REJECT_RETURN;

			sfd.timecodeFragmentData(stf);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
		case 'C': // CropFragment and CCFragment
			if (typeName[1] == 'r') { // assume CropFragment
				c->status = napi_get_named_property(env, prop, "x", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &scf.x);
				REJECT_RETURN;

				c->status = napi_get_named_property(env, prop, "y", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &scf.y);
				REJECT_RETURN;

				c->status = napi_get_named_property(env, prop, "width", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &scf.width);
				REJECT_RETURN;

				c->status = napi_get_named_property(env, prop, "height", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &scf.height);
				REJECT_RETURN;

				sfd.cropFragmentData(scf);
				sf.fragmentData = sfd;
				c->fragments[fragmentCount++] = sf;
			} else { // assume ccFragment
				c->status = napi_get_named_property(env, prop, "ccID", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_string_utf8(env, subprop, rushID, 33, nullptr);
				REJECT_RETURN;
				rushIDStr.assign(rushID);
				sccf.ccID = {
					(CORBA::LongLong) strtoull(rushIDStr.substr(0, 16).c_str(), nullptr, 16),
					(CORBA::LongLong) strtoull(rushIDStr.substr(16, 32).c_str(), nullptr, 16) };

				c->status = napi_get_named_property(env, prop, "ccType", &subprop);
				REJECT_RETURN;
				c->status = napi_get_value_int32(env, subprop, (int32_t *) &sccf.ccType);
				REJECT_RETURN;

				// TODO RawData type parameter "ccData"

				sfd.ccFragmentData(sccf);
				sf.fragmentData = sfd;
				c->fragments[fragmentCount++] = sf;
			}
			break;
		case 'S': // SpeedFragment or ServerFragment
			if (typeName[0] != 'p') { break; }
			c->status = napi_get_named_property(env, prop, "speed", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &spdf.speed);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "profile", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &spdf.profile);
			REJECT_RETURN;

			sfd.speedFragmentData(spdf);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
		case 'E': // EffectFragment
			c->status = napi_get_named_property(env, prop, "effectID", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &seff.effectID);
			REJECT_RETURN;

			// TODO RawData type parameter "effectData"

			sfd.effectFragmentData(seff);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
		case 'N': // NoteFragment
			c->status = napi_get_named_property(env, prop, "noteID", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &sntf.noteID);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "aux", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &sntf.aux);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "mask", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &sntf.mask);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "note", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_string_utf8(env, subprop, nullptr, 0, &noteLen);
			REJECT_RETURN;
			noteStr = (char*) malloc((noteLen + 1) * sizeof(char));
			c->status = napi_get_value_string_utf8(env, subprop, noteStr, noteLen + 1, &noteLen);
			REJECT_RETURN;
			sntf.note = utf8_conv.from_bytes(noteStr).data();

			sfd.noteFragmentData(sntf);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
		case 'P': // PanZoomFragment
			c->status = napi_get_named_property(env, prop, "x", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &spzf.x);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "y", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &spzf.y);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "hZoom", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &spzf.hZoom);
			REJECT_RETURN;

			c->status = napi_get_named_property(env, prop, "vZoom", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &spzf.vZoom);
			REJECT_RETURN;

			sfd.panZoomFragmentData(spzf);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
		case 'F': // FlagsFragment
			c->status = napi_get_named_property(env, prop, "flags", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &sff.flags);
			REJECT_RETURN;

			sfd.flagsFragmentData(sff);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
		case 'M': // MultiCamFragment
			c->status = napi_get_named_property(env, prop, "stream", &subprop);
			REJECT_RETURN;
			c->status = napi_get_value_int32(env, subprop, (int32_t *) &smcf.stream);
			REJECT_RETURN;

			sfd.multiCamFragmentData(smcf);
			sf.fragmentData = sfd;
			c->fragments[fragmentCount++] = sf;
			break;
    default:
      break;
    } // switch typeName[0]
  } // for loop through incoming fragments
  c->fragments.length(fragmentCount);

	c->status = napi_create_string_utf8(env, "LoadPlayPort", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, loadPlayPortExecute,
    loadPlayPortComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void getPortFragmentsExecute(napi_env env, void* data) {
	portFragmentsCarrier* c = (portFragmentsCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		Quentin::Server_ptr server = zp->getServer(c->serverID);
		Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

		c->fragments = port->getPortFragments(c->start, c->finish);
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
}

void getPortFragmentsComplete(napi_env env, napi_status asyncStatus, void* data) {
	portFragmentsCarrier* c = (portFragmentsCarrier*) data;
	napi_value result, prop;
	// char rushID[33];

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Get fragments failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;
	c->status = napi_create_string_utf8(env, "ServerFragments", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->serverID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "serverID", prop);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, c->portName.c_str(), NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "portName", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, -1, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "clipID", prop);
	REJECT_STATUS;

	if (c->start >= 0) {
		c->status = napi_create_int32(env, c->start, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, result, "start", prop);
		REJECT_STATUS;
	}

	if (c->finish >= 0) {
		c->status = napi_create_int32(env, c->finish, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, result, "finish", prop);
		REJECT_STATUS;
	}

	c->status = fragmentsToJS(env, c->fragments, &prop);
	REJECT_STATUS;
	c->status =  napi_set_named_property(env, result, "fragments", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getPortFragments(napi_env env, napi_callback_info info) {
	portFragmentsCarrier* c = new portFragmentsCarrier;
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
		REJECT_ERROR_RETURN("Get port fragments must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

  if (argc < 2) {
    REJECT_ERROR_RETURN("Options object with serverID and port name must be provided.",
			QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with a server ID and port name.",
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

	c->status = napi_get_named_property(env, options, "start", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_number) {
		c->status = napi_get_value_int32(env, prop, &c->start);
		REJECT_RETURN;
	}

	c->status = napi_get_named_property(env, options, "finish", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_number) {
		c->status = napi_get_value_int32(env, prop, &c->finish);
		REJECT_RETURN;
	}

	c->status = napi_create_string_utf8(env, "GetPortFragments", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, getPortFragmentsExecute,
    getPortFragmentsComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void getPortPropertiesExecute(napi_env env, void* data) {
	portPropertiesCarrier* c = (portPropertiesCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		Quentin::Server_ptr server = zp->getServer(c->serverID);
		Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

		printf("Port properties execute ... here we go\n");
		Quentin::WStrings_var propList = port->getPropertyList();
		for ( uint32_t x = 0 ; x < propList->length() ; x++ ) {
			CORBA::WChar* value = port->getProperty(propList[x]);
			c->properties.emplace(
				utf8_conv.to_bytes(propList[x]),
				utf8_conv.to_bytes(value));
		}
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
}

void getPortPropertiesComplete(napi_env env, napi_status asyncStatus, void* data) {
	portPropertiesCarrier* c = (portPropertiesCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Port properties failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;
	c->status = napi_create_string_utf8(env, "PortProperties", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	for ( auto it = c->properties.begin(); it != c->properties.end() ; ++it ) {
		c->status = napi_create_string_utf8(env, it->second.c_str(), NAPI_AUTO_LENGTH, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, result, it->first.c_str(), prop);
		REJECT_STATUS;
	}

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getPortProperties(napi_env env, napi_callback_info info) {
	portPropertiesCarrier* c = new portPropertiesCarrier;
	napi_value promise, resourceName, options, prop;
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
		REJECT_ERROR_RETURN("Port properties must be provided with a IOR reference to an ISA server.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with server ID and port name must be provided.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with server ID and port name.",
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

	c->status = napi_create_string_utf8(env, "GetPortProperties", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, getPortPropertiesExecute,
		getPortPropertiesComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

	return promise;
}
