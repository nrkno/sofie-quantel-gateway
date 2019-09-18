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

#include "clone.h"

// Deprecated ... cloneInterZone is now a general clone operation
void cloneIfNeededExecute(napi_env env, void* data) {
	cloneIfNeededCarrier* c = (cloneIfNeededCarrier*) data;
	Quentin::ZonePortal_ptr zpp;
	Quentin::ZonePortal_var zpv;
	CORBA::Boolean copyCreated;

	try {
		resolveZonePortalShared(c->isaIOR, &zpp);
		zpv = zpp;

		zpv->cloneIfNeeded(c->clipID, c->poolID, 0,
			c->highPriority ? Quentin::Port::HighPriority : Quentin::Port::StandardPriority,
			-1, copyCreated); // TODO -1 means the cloned clip never expires. Is this OK?

		c->copyCreated = copyCreated;
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

void cloneIfNeededComplete(napi_env env, napi_status asyncStatus, void* data) {
	cloneIfNeededCarrier* c = (cloneIfNeededCarrier*) data;
	napi_value result;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Clone if needed failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->copyCreated, &result);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value cloneIfNeeded(napi_env env, napi_callback_info info) {
	cloneIfNeededCarrier* c = new cloneIfNeededCarrier;
	napi_value promise, prop, resourceName;
	napi_valuetype type;
	bool isArray;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

  c->status = napi_create_promise(env, &c->_deferred, &promise);
  REJECT_RETURN;

	size_t argc = 2;
	napi_value argv[2];
	c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Clone if needed must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with clip ID and pool ID must be provided.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with clip ID and pool ID.",
			QGW_INVALID_ARGS);
	}

	c->status = napi_get_named_property(env, argv[1], "clipID", &prop);
	REJECT_RETURN;
	c->status = napi_get_value_int32(env, prop, &c->clipID);
	REJECT_RETURN;

	c->status = napi_get_named_property(env, argv[1], "poolID", &prop);
	REJECT_RETURN;
	c->status = napi_get_value_int32(env, prop, &c->poolID);
	REJECT_RETURN;

	c->status = napi_get_named_property(env, argv[1], "highPriority", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_boolean) {
		c->status = napi_get_value_bool(env, prop, &c->highPriority);
		REJECT_RETURN;
	}

	c->status = napi_create_string_utf8(env, "CloneIfNeeded", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, cloneIfNeededExecute,
    cloneIfNeededComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

	return promise;
}

void cloneInterZoneExecute(napi_env env, void* data) {
	cloneInterZoneCarrier* c = (cloneInterZoneCarrier*) data;
	Quentin::ZonePortal_ptr zpp;
	Quentin::ZonePortal_var zpv;
	CORBA::Boolean copyCreated;

	try {
		resolveZonePortalShared(c->isaIOR, &zpp);
		zpv = zpp;

		if (c->zoneID < 0) { // Local zone clone
			// Note: Clip does not expire - that's what -1 means!
			c->copyID = zpv->cloneIfNeeded(c->clipID, c->poolID, 0, c->priority, -1, copyCreated);
			c->copyCreated = copyCreated;
		} else {
			if (c->history) {
				c->copyID = zpv->cloneClipInterZone(c->zoneID, c->clipID, c->poolID, c->priority);
			} else {
				c->copyID = zpv->cloneClipInterZoneWithoutHistory(c->zoneID, c->clipID, c->poolID, c->priority);
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

void cloneInterZoneComplete(napi_env env, napi_status asyncStatus, void* data) {
	cloneInterZoneCarrier* c = (cloneInterZoneCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Clone inter zone failed to complete.";
	}
	REJECT_STATUS;

  c->status = napi_create_object(env, &result);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "CloneResult", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	if (c->zoneID >= 0) {
		c->status = napi_create_int32(env, c->zoneID , &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, result, "zoneID", prop);
		REJECT_STATUS;
	}

	c->status = napi_create_int32(env, c->clipID , &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "clipID", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->poolID , &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "poolID", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->priority , &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "priority", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->history, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "history", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->copyCreated, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "copyCreated", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->copyID, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "copyID", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value cloneInterZone(napi_env env, napi_callback_info info) {
	cloneInterZoneCarrier* c = new cloneInterZoneCarrier;
	napi_value promise, resourceName, prop, options;
	napi_valuetype type;
	bool isArray;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

	c->status = napi_create_promise(env, &c->_deferred, &promise);
	REJECT_RETURN;

	size_t argc = 2;
	napi_value argv[2];
	c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Clone inter zone must be provided with a IOR reference to an ISA server.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Clone inter zone must be provided with an options object containing source zone ID, source clip ID and destination pool ID.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with a source zone ID, source clip ID and destination pool ID.",
			QGW_INVALID_ARGS);
	}

	options = argv[1];
	c->status = napi_get_named_property(env, options, "zoneID", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_number) {
		c->status = napi_get_value_int32(env, prop, (int32_t*) &c->zoneID);
		REJECT_RETURN;
	}

	c->status = napi_get_named_property(env, options, "clipID", &prop);
	REJECT_RETURN;
	c->status = napi_get_value_int32(env, prop, (int32_t*) &c->clipID);
	REJECT_RETURN;

	c->status = napi_get_named_property(env, options, "poolID", &prop);
	REJECT_RETURN;
	c->status = napi_get_value_int32(env, prop, (int32_t*) &c->poolID);
	REJECT_RETURN;

	c->status = napi_get_named_property(env, options, "priority", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_number) {
		c->status = napi_get_value_int32(env, prop, (int32_t*) &c->priority);
		REJECT_RETURN;
	}

	c->status = napi_get_named_property(env, options, "history", &prop);
	REJECT_RETURN;
	c->status = napi_typeof(env, prop, &type);
	REJECT_RETURN;
	if (type == napi_boolean) {
		c->status = napi_get_value_bool(env, prop, &c->history);
		REJECT_RETURN;
	}

	c->status = napi_create_string_utf8(env, "CloneInterZone", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, cloneInterZoneExecute,
		cloneInterZoneComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

	return promise;
}

void getCopyRemainingExecute(napi_env env, void* data) {
	getCopyRemainingCarrier *c = (getCopyRemainingCarrier*) data;
	Quentin::ZonePortal_ptr zpp;
	Quentin::ZonePortal_var zpv;

	try {
		resolveZonePortalShared(c->isaIOR, &zpp);
		zpv = zpp;

		c->progress = zpv->getCopyRemaining(c->clipID);
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

void getCopyRemainingComplete(napi_env env, napi_status asyncStatus, void* data){
	getCopyRemainingCarrier *c = (getCopyRemainingCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Get copy remaining failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "CopyProgress", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->progress.clipID , &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "clipID", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->progress.totalProtons, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "totalProtons", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->progress.protonsLeft, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "protonsLeft", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->progress.secsLeft, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "secsLeft", prop);
	REJECT_STATUS;

	c->status = napi_create_int32(env, c->progress.priority, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "priority", prop);
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->progress.ticketed, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "ticketed", prop);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getCopyRemaining(napi_env env, napi_callback_info info) {
	getCopyRemainingCarrier* c = new getCopyRemainingCarrier;
	napi_value promise, resourceName, prop, options;
	napi_valuetype type;
	bool isArray;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

	c->status = napi_create_promise(env, &c->_deferred, &promise);
	REJECT_RETURN;

	size_t argc = 2;
	napi_value argv[2];
	c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Get copy remaining must be provided with a IOR reference to an ISA server.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Get copy remaining must be provided with an options object containing a clip ID.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with a clip ID.",
			QGW_INVALID_ARGS);
	}

	options = argv[1];
	c->status = napi_get_named_property(env, options, "clipID", &prop);
	REJECT_RETURN;
	c->status = napi_get_value_int32(env, prop, (int32_t*) &c->clipID);
	REJECT_RETURN;

	c->status = napi_create_string_utf8(env, "GetCopyRemaining", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, getCopyRemainingExecute,
		getCopyRemainingComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

	return promise;
}

void getCopiesRemainingExecute(napi_env env, void* data) {
	getCopiesRemainingCarrier* c = (getCopiesRemainingCarrier*) data;
	Quentin::ZonePortal_ptr zpp;
	Quentin::ZonePortal_var zpv;
	Quentin::CopyProgressList_var cpl;

	try {
		resolveZonePortalShared(c->isaIOR, &zpp);
		zpv = zpp;

		cpl = zpv->getCopiesRemaining();
		for ( uint32_t x = 0 ; x < cpl->length() ; x++ ) {
			c->progresses.push_back(cpl[x]);
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

void getCopiesRemainingComplete(napi_env env, napi_status asyncStatus, void* data) {
	getCopiesRemainingCarrier* c = (getCopiesRemainingCarrier*) data;
	napi_value result, prog, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Get copies remaining failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_array(env, &result);
	REJECT_STATUS;

	for ( uint32_t x = 0 ; x < c->progresses.size() ; x++ ) {
		c->status = napi_create_object(env, &prog);
		REJECT_STATUS;
		Quentin::CopyProgress cp = c->progresses.at(x);

		c->status = napi_create_string_utf8(env, "CopyProgress", NAPI_AUTO_LENGTH, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "type", prop);
		REJECT_STATUS;

		c->status = napi_create_int32(env, cp.clipID , &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "clipID", prop);
		REJECT_STATUS;

		c->status = napi_create_int32(env, cp.totalProtons, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "totalProtons", prop);
		REJECT_STATUS;

		c->status = napi_create_int32(env, cp.protonsLeft, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "protonsLeft", prop);
		REJECT_STATUS;

		c->status = napi_create_int32(env, cp.secsLeft, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "secsLeft", prop);
		REJECT_STATUS;

		c->status = napi_create_int32(env, cp.priority, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "priority", prop);
		REJECT_STATUS;

		c->status = napi_get_boolean(env, cp.ticketed, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, prog, "ticketed", prop);
		REJECT_STATUS;

		c->status = napi_set_element(env, result, x, prog);
		REJECT_STATUS;
	}

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getCopiesRemaining(napi_env env, napi_callback_info info) {
	getCopiesRemainingCarrier *c = new getCopiesRemainingCarrier;
	napi_value promise, resourceName;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

	c->status = napi_create_promise(env, &c->_deferred, &promise);
	REJECT_RETURN;

	size_t argc = 1;
	napi_value argv[1];
	c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Get copies remaining must be provided with a IOR reference to an ISA server.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	c->status = napi_create_string_utf8(env, "GetCopiesRemaining", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, getCopiesRemainingExecute,
		getCopiesRemainingComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

	return promise;
}
