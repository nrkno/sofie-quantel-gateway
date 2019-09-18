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

#include "control.h"

void triggerExecute(napi_env env, void* data) {
	triggerCarrier* c = (triggerCarrier*) data;
	Quentin::ZonePortal_ptr zpp;
	Quentin::ZonePortal_var zpv;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zpp);
		zpv = zpp;

    Quentin::Server_ptr server = zpv->getServer(c->serverID);
    Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

    if (!port->setTrigger(c->trigger,
      	c->offset >= 0 ? Quentin::Port::trModeOffset : Quentin::Port::trModeNow,
      	c->offset >= 0 ? c->offset : 0) ) {
      if (c->offset >= 0) {
				c->status = QGW_TRIGGER_OFFSET_FAIL;
        c->errorMsg = "Failed to trigger action at offset.";
				return;
      } else {
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
}

void triggerComplete(napi_env env, napi_status asyncStatus, void* data) {
	triggerCarrier* c = (triggerCarrier*) data;
	napi_value result;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Trigger failed to complete.";
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

void jumpExecute(napi_env env, void* data) {
	jumpCarrier* c = (jumpCarrier*) data;
	Quentin::ZonePortal_ptr zpp;
	Quentin::ZonePortal_var zpv;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zpp);
		zpv = zpp;

		Quentin::Server_ptr server = zpv->getServer(c->serverID);
		Quentin::Port_ptr port = server->getPort(utf8_conv.from_bytes(c->portName).data(), 0);

		if (c->hardJump) {
			port->jump(c->offset, false);
		} else {
			port->setJump(c->offset);
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

void jumpComplete(napi_env env, napi_status asyncStatus, void* data) {
	jumpCarrier* c = (jumpCarrier*) data;
	napi_value result;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Jump failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_get_boolean(env, true, &result);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value qJump(napi_env env, napi_callback_info info) {
	return coreJump(env, info, true);
}

napi_value setJump(napi_env env, napi_callback_info info) {
	return coreJump(env, info, false);
}

napi_value coreJump(napi_env env, napi_callback_info info, bool hardJump) {
  jumpCarrier* c = new jumpCarrier;
  napi_value promise, prop, options, resourceName;
  napi_valuetype type;
  bool isArray;
  char* portName;
  size_t portNameLen;
	char* isaIOR = nullptr;
	size_t iorLen = 0;
	c->hardJump = hardJump;

  c->status = napi_create_promise(env, &c->_deferred, &promise);
  REJECT_RETURN;

  size_t argc = 2;
  napi_value argv[2];
  c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Jump must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

  if (argc < 2) {
    REJECT_ERROR_RETURN("Options object with server ID, port name and jump offset must be provided.",
			QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with server ID, port name and jump offset.",
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

	c->status = napi_create_string_utf8(env, c->hardJump ? "HardJump" : "SoftJump",
	  NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, jumpExecute,
		jumpComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

  return promise;
}
