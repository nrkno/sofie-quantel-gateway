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

#include "clip.h"

void getClipDataExecute(napi_env env, void* data) {
  clipDataCarrier* c = (clipDataCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::WStrings columns;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);
		Quentin::ColumnDescList_var cdl = zp->getColumnDescriptions();
		columns.length(cdl->length());

		for ( uint32_t x = 0 ; x < cdl->length() ; x++ ) {
			columns[x] = cdl[x].columnName;
			c->columnNames.push_back(utf8_conv.to_bytes(cdl[x].columnName));
			c->columnTypes.push_back(utf8_conv.to_bytes(cdl[x].columnType));
		}

		Quentin::WStrings_var results = zp->getClipData(c->clipID, columns);

		for ( uint32_t i = 0 ; i < results->length() ; i++ ) {
			c->values.push_back(utf8_conv.to_bytes(results[i]));
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

void getClipDataComplete(napi_env env, napi_status asyncStatus, void* data) {
	clipDataCarrier* c = (clipDataCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Get clip data failed to complete.";
	}
	REJECT_STATUS;

	napi_create_object(env, &result);
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "ClipData", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, result, "type", prop);
	REJECT_STATUS;

	for ( uint32_t x = 0 ; x < c->values.size() ; x++ ) {
		c->status = napi_create_string_utf8(env, c->values.at(x).c_str(), NAPI_AUTO_LENGTH, &prop);
		REJECT_STATUS;

		if (c->columnTypes.at(x) == booleanName) {
			c->status = napi_get_boolean(env, c->values.at(x) == "1", &prop);
			REJECT_STATUS;
		}

		if (c->columnTypes.at(x) == numberName) {
			if (c->values.at(x).length() > 0) {
				c->status = napi_create_int32(env, std::stol(c->values.at(x)), &prop);
				REJECT_STATUS;
			} else {
				c->status = napi_get_null(env, &prop);
				REJECT_STATUS;
			}
		};

		if (c->columnTypes.at(x) == dateName) {
			if (c->values.at(x).length() > 0) {
				c->status = convertToDate(env, c->values.at(x), &prop);
				REJECT_STATUS;
			} else {
				c->status = napi_get_null(env, &prop);
				REJECT_STATUS;
			}
		}

		c->status = napi_set_named_property(env, result, c->columnNames.at(x).c_str(), prop);
		REJECT_STATUS;
	}

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getClipData(napi_env env, napi_callback_info info) {
	clipDataCarrier* c = new clipDataCarrier;
  napi_value promise, prop, options, resourceName;
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
		REJECT_ERROR_RETURN("Clip data request must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with clip ID must be provided.",
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
	c->status = napi_get_value_int32(env, prop, &c->clipID);
	REJECT_RETURN;

	c->status = napi_create_string_utf8(env, "GetClipData", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, getClipDataExecute,
		getClipDataComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

  return promise;
}

void searchClipsExecute(napi_env env, void* data) {
	searchClipsCarrier* c = (searchClipsCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::ClipPropertyList cpl;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	uint32_t queryCounter = 0;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		cpl.length(c->query.size());
		for ( auto it = c->query.begin(); it != c->query.end() ; ++it ) {
			Quentin::ClipProperty cp = { it->first.data(), it->second.data() };
			cpl[queryCounter++] = cp;
		}

		Quentin::WStrings columnNamesWide;
		columnNamesWide.length(c->colNames.size());
		for ( uint32_t i = 0 ; i < c->colNames.size() ; i++ ) {
			columnNamesWide[i] = utf8_conv.from_bytes(c->colNames.at(i)).data();
		}

  	Quentin::WStrings_var results = zp->searchClips(cpl, columnNamesWide, c->limit);

  	for ( uint32_t i = 0 ; i < results->length() ; i++ ) {
			c->values.push_back(utf8_conv.to_bytes(results[i]));
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

void searchClipsComplete(napi_env env, napi_status asyncStatus, void* data) {
	searchClipsCarrier* c = (searchClipsCarrier*) data;
	napi_value result, term, prop;
	uint32_t resultCount = 0;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Search clips failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_array(env, &result);
	REJECT_STATUS;

	if (c->idOnly) {
		for ( uint32_t x = 0 ; x < c->values.size() ; x++ ) {
			c->status = napi_create_int32(env, std::stol(c->values.at(x)), &prop);
			REJECT_STATUS;
			c->status = napi_set_element(env, result, x, prop);
			REJECT_STATUS;
		}
	} else {
		c->status = napi_create_object(env, &term);
		REJECT_STATUS;
		c->status = napi_create_string_utf8(env, "ClipDataSummary", NAPI_AUTO_LENGTH, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, term, "type", prop);
		REJECT_STATUS;

		for ( uint32_t x = 0 ; x < c->values.size() ; x++ ) {
			std::string value = c->values.at(x);
			std::string key = c->colNames.at(x % c->colNames.size());

			if ((key == "ClipID") || (key == "CloneId") || (key == "PoolID")) {
        if (value.length() == 0) {
          c->status = napi_get_null(env, &prop);
        } else {
    			c->status = napi_create_int32(env, std::stol(value), &prop);
        }
    		REJECT_STATUS;
			} else if ((key == "Completed") || (key == "Created")) {
				c->status = convertToDate(env, value, &prop);
				REJECT_STATUS;
			} else {
				c->status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &prop);
				REJECT_STATUS;
			}
			c->status = napi_set_named_property(env, term, key.c_str(), prop);
			REJECT_STATUS;

			if (x % c->colNames.size() == (c->colNames.size() - 1)) {
				c->status = napi_set_element(env, result, resultCount++, term);
				REJECT_STATUS;
				if (x < c->values.size() - 2) {
					c->status = napi_create_object(env, &term);
					REJECT_STATUS;
					c->status = napi_create_string_utf8(env, "ClipDataSummary", NAPI_AUTO_LENGTH, &prop);
					REJECT_STATUS;
					c->status = napi_set_named_property(env, term, "type", prop);
					REJECT_STATUS;
				}
			}
		}
	}

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value searchClips(napi_env env, napi_callback_info info) {
	searchClipsCarrier* c = new searchClipsCarrier;
	napi_value promise, prop, options, term, terms, resourceName;
	napi_valuetype type;
	bool isArray;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	uint32_t termCount;
	char* nameStr;
	char* valueStr;
	size_t strLen;

	char* isaIOR = nullptr;
	size_t iorLen = 0;

  c->status = napi_create_promise(env, &c->_deferred, &promise);
  REJECT_RETURN;

	size_t argc = 2;
	napi_value argv[2];
	c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Clip search must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with search terms must be provided.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with search terms.",
	    QGW_INVALID_ARGS);
	}

	options = argv[1];
	c->status = napi_get_property_names(env, options, &terms);
	REJECT_RETURN;
	c->status = napi_get_array_length(env, terms, &termCount);
	REJECT_RETURN;
	for ( uint32_t x = 0 ; x < termCount ; x++ ) {
		c->status = napi_get_element(env, terms, x, &term);
		REJECT_RETURN;
		c->status = napi_get_property(env, options, term, &prop);
		REJECT_RETURN;

		c->status = napi_get_value_string_utf8(env, term, nullptr, 0, &strLen);
		REJECT_RETURN;
		nameStr = (char *) malloc((strLen + 1) * sizeof(char));
		c->status = napi_get_value_string_utf8(env, term, nameStr, strLen + 1, &strLen);
		REJECT_RETURN;

		if (std::string(nameStr) == "limit") {
			c->status = napi_get_value_int32(env, prop, &c->limit);
			REJECT_RETURN;
		} else if (std::string(nameStr) == "idOnly") {
			c->colNames = clipIDonly;
			c->idOnly = true;
		} else {
			c->status = napi_get_value_string_utf8(env, prop, nullptr, 0, &strLen);
			REJECT_RETURN;
			valueStr = (char *) malloc((strLen + 1) * sizeof(char));
			c->status = napi_get_value_string_utf8(env, prop, valueStr, strLen + 1, &strLen);
			REJECT_RETURN;

			c->query.emplace(
				utf8_conv.from_bytes(std::string(nameStr)),
				utf8_conv.from_bytes(std::string(valueStr)));
				free(valueStr);
		}
		free(nameStr);
	}

	c->status = napi_create_string_utf8(env, "SearchClips", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, searchClipsExecute,
    searchClipsComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

	return promise;
}

void getFragmentsExecute(napi_env env, void* data) {
	getFragmentsCarrier* c = (getFragmentsCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		c->fragments = (c->start >= 0) && (c->finish >= 0) ?
		  zp->getFragments(c->clipID, c->start, c->finish) : zp->getAllFragments(c->clipID);
		// Turns out this is not needed. If clip has decent timecode, you will get a TimecodeFragment
		// c->sourceTCs = zp->getSourceTimecode(c->clipID, c->start, c->finish);
		// c->refTCs = zp->getSourceTimecode(c->clipID, c->start, c->finish);
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

void getFragmentsComplete(napi_env env, napi_status asyncStatus, void* data) {
	getFragmentsCarrier* c = (getFragmentsCarrier*) data;
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

	c->status = napi_create_int32(env, c->clipID, &prop);
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
	// Not needed - if decenet clip timecode, it is provided as a fragment
	// c->status = fragmentsToJS(env, c->sourceTCs, &prop);
	// REJECT_STATUS;
	// c->status = fragmentsToJS(env, c->refTCs, &prop);
	// REJECT_STATUS;
  c->status =  napi_set_named_property(env, result, "fragments", prop);
  REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getFragments(napi_env env, napi_callback_info info) {
  getFragmentsCarrier* c = new getFragmentsCarrier;
  napi_value promise, prop, options, resourceName;
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
		REJECT_ERROR_RETURN("Get fragments must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

  if (argc < 2) {
    REJECT_ERROR_RETURN("Options object with clip ID and optional start and finish must be provided.",
			QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with a clip ID and optional start/finish.",
			QGW_INVALID_ARGS);
  }

  options = argv[1];
  c->status = napi_get_named_property(env, options, "clipID", &prop);
  REJECT_RETURN;
  c->status = napi_get_value_int32(env, prop, &c->clipID);
  REJECT_RETURN;

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

	c->status = napi_create_string_utf8(env, "GetFragments", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, getFragmentsExecute,
    getFragmentsComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void deletClipExecute(napi_env env, void* data) {
	deleteClipCarrier* c = (deleteClipCarrier*) data;
	Quentin::ZonePortal::_ptr_type zp;

	try {
		resolveZonePortalShared(c->isaIOR, &zp);

		c->deleted = zp->deleteClip(c->clipID);
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

void deleteClipComplete(napi_env env, napi_status asyncStatus, void* data) {
	deleteClipCarrier* c = (deleteClipCarrier*) data;
	napi_value result;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Test connection failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_get_boolean(env, c->deleted, &result);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value deleteClip(napi_env env, napi_callback_info info) {
	deleteClipCarrier* c = new deleteClipCarrier;
	napi_valuetype type;
	bool isArray;
	napi_value promise, prop, resourceName;
	char* isaIOR = nullptr;
	size_t iorLen = 0;

	c->status = napi_create_promise(env, &c->_deferred, &promise);
	REJECT_RETURN;

	size_t argc = 2;
	napi_value argv[2];
	c->status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
	REJECT_RETURN;

	if (argc < 1) {
		REJECT_ERROR_RETURN("Delete clip must be provided with a IOR reference to an ISA server.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	if (argc < 2) {
		REJECT_ERROR_RETURN("Options object with a clip ID must be provided.",
			QGW_INVALID_ARGS);
	}
	c->status = napi_typeof(env, argv[1], &type);
	REJECT_RETURN;
	c->status = napi_is_array(env, argv[1], &isArray);
	REJECT_RETURN;
	if (isArray || type != napi_object) {
		REJECT_ERROR_RETURN("Argument must be an options object with clip ID.",
			QGW_INVALID_ARGS);
	}

	c->status = napi_get_named_property(env, argv[1], "clipID", &prop);
	REJECT_RETURN;
	c->status = napi_get_value_int32(env, prop, &c->clipID);
	REJECT_RETURN;

	c->status = napi_create_string_utf8(env, "DeleteClip", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, deletClipExecute,
		deleteClipComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

	return promise;
}
