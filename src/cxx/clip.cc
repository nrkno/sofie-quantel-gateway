#include "clip.h"

void getClipDataExecute(napi_env env, void* data) {
  clipDataCarrier* c = (clipDataCarrier*) data;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::WStrings columns;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);
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

	orb->destroy();
}

void getClipDataComplete(napi_env env, napi_status asyncStatus, void* data) {
	clipDataCarrier* c = (clipDataCarrier*) data;
	napi_value result, prop;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Test connection failed to complete.";
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
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::ClipPropertyList cpl;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	uint32_t queryCounter = 0;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);

		cpl.length(c->query.size());
		for ( auto it = c->query.begin(); it != c->query.end() ; ++it ) {
			Quentin::ClipProperty cp = { it->first.data(), it->second.data() };
			cpl[queryCounter++] = cp;
		}

		Quentin::WStrings columnNamesWide;
		columnNamesWide.length(columnNames.size());
		for ( uint32_t i = 0 ; i < columnNames.size() ; i++ ) {
			columnNamesWide[i] = utf8_conv.from_bytes(columnNames.at(i)).data();
		}

		Quentin::WStrings_var results = zp->searchClips(cpl, columnNamesWide, 10);

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

	orb->destroy();
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

	c->status = napi_create_object(env, &term);
	REJECT_STATUS;
	c->status = napi_create_string_utf8(env, "ClipDataSummary", NAPI_AUTO_LENGTH, &prop);
	REJECT_STATUS;
	c->status = napi_set_named_property(env, term, "type", prop);
	REJECT_STATUS;

	for ( uint32_t x = 0 ; x < c->values.size() ; x++ ) {
		std::string value = c->values.at(x);
		std::string key = columnNames.at(x % columnNames.size());

		if ((key == "ClipID") || (key == "CloneID") || (key == "PoolID")) {
			c->status = napi_create_int32(env, std::stol(value), &prop);
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

		if (x % columnNames.size() == (columnNames.size() - 1)) {
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

		c->status = napi_get_value_string_utf8(env, prop, nullptr, 0, &strLen);
		REJECT_RETURN;
		valueStr = (char *) malloc((strLen + 1) * sizeof(char));
		c->status = napi_get_value_string_utf8(env, prop, valueStr, strLen + 1, &strLen);
		REJECT_RETURN;

		c->query.emplace(
			utf8_conv.from_bytes(std::string(nameStr)),
			utf8_conv.from_bytes(std::string(valueStr)));
		free(nameStr);
		free(valueStr);
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
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);

		c->fragments = (c->start >= 0) && (c->finish >= 0) ?
		  zp->getFragments(c->clipID, c->start, c->finish) : zp->getAllFragments(c->clipID);
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

void getFragmentsComplete(napi_env env, napi_status asyncStatus, void* data) {
	getFragmentsCarrier* c = (getFragmentsCarrier*) data;
	napi_value result, frag, prop, fragprop;
	char rushID[33];

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Test connection failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_object(env, &result);
	REJECT_STATUS;
	c->status = napi_create_string_utf8(env, "ServerFramgments", NAPI_AUTO_LENGTH, &prop);
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

	c->status = napi_create_array(env, &prop);
  REJECT_STATUS;

  for ( uint32_t x = 0 ; x < c->fragments->length() ; x++ ) {
    c->status =  napi_create_object(env, &frag);
    REJECT_STATUS;

    switch (c->fragments[x].fragmentData._d()) {
    case Quentin::FragmentType::videoFragment:
      c->status =  napi_create_string_utf8(env, "VideoFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::audioFragment:
      c->status =  napi_create_string_utf8(env, "AudioFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::auxFragment:
      c->status =  napi_create_string_utf8(env, "AUXFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::flagsFragment:
      c->status =  napi_create_string_utf8(env, "FlagsFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::timecodeFragment:
      c->status =  napi_create_string_utf8(env, "TimecodeFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::cropFragment:
      c->status =  napi_create_string_utf8(env, "CropFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::panZoomFragment:
      c->status =  napi_create_string_utf8(env, "PanZoomFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::speedFragment:
      c->status =  napi_create_string_utf8(env, "SpeedFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::multiCamFragment:
      c->status =  napi_create_string_utf8(env, "MultiCamFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::ccFragment:
      c->status =  napi_create_string_utf8(env, "CCFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::effectFragment:
      c->status =  napi_create_string_utf8(env, "EffectFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    default:
      c->status =  napi_create_string_utf8(env, "ServerFragment", NAPI_AUTO_LENGTH, &fragprop);
      REJECT_STATUS;
      break;
    }
    c->status =  napi_set_named_property(env, frag, "type", fragprop);
    REJECT_STATUS;

    c->status =  napi_create_int32(env, c->fragments[x].trackNum, &fragprop);
    REJECT_STATUS;
    c->status =  napi_set_named_property(env, frag, "trackNum", fragprop);
    REJECT_STATUS;

    c->status =  napi_create_int32(env, c->fragments[x].start, &fragprop);
    REJECT_STATUS;
    c->status =  napi_set_named_property(env, frag, "start", fragprop);
    REJECT_STATUS;

    c->status =  napi_create_int32(env, c->fragments[x].finish, &fragprop);
    REJECT_STATUS;
    c->status =  napi_set_named_property(env, frag, "finish", fragprop);
    REJECT_STATUS;

    switch (c->fragments[x].fragmentData._d()) {
    case Quentin::FragmentType::videoFragment:
      sprintf(rushID, "%016llx%016llx",
        c->fragments[x].fragmentData.videoFragmentData().rushID.first,
        c->fragments[x].fragmentData.videoFragmentData().rushID.second);
      c->status =  napi_create_string_utf8(env, rushID, 32, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "rushID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.videoFragmentData().format, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "format", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.videoFragmentData().poolID, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "poolID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int64(env, c->fragments[x].fragmentData.videoFragmentData().poolFrame, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "poolFrame", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.videoFragmentData().skew, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "skew", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.videoFragmentData().rushFrame, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "rushFrame", fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::audioFragment:
      sprintf(rushID, "%016llx%016llx",
        c->fragments[x].fragmentData.audioFragmentData().rushID.first,
        c->fragments[x].fragmentData.audioFragmentData().rushID.second);
      c->status =  napi_create_string_utf8(env, rushID, 32, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "rushID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.audioFragmentData().format, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "format", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.audioFragmentData().poolID, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "poolID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int64(env, c->fragments[x].fragmentData.audioFragmentData().poolFrame, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "poolFrame", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.audioFragmentData().skew, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "skew", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.audioFragmentData().rushFrame, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "rushFrame", fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::auxFragment:
      sprintf(rushID, "%016llx%016llx",
        c->fragments[x].fragmentData.auxFragmentData().rushID.first,
        c->fragments[x].fragmentData.auxFragmentData().rushID.second);
      c->status =  napi_create_string_utf8(env, rushID, 32, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "rushID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.auxFragmentData().format, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "format", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.auxFragmentData().poolID, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "poolID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int64(env, c->fragments[x].fragmentData.auxFragmentData().poolFrame, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "poolFrame", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.auxFragmentData().skew, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "skew", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.auxFragmentData().rushFrame, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "rushFrame", fragprop);
      REJECT_STATUS;
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
        c->fragments[x].fragmentData.ccFragmentData().ccID.first,
        c->fragments[x].fragmentData.ccFragmentData().ccID.second);
      c->status =  napi_create_string_utf8(env, rushID, 32, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "ccID", fragprop);
      REJECT_STATUS;

      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.ccFragmentData().ccType, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "ccType", fragprop);
      REJECT_STATUS;
      break;
    case Quentin::FragmentType::effectFragment:
      c->status =  napi_create_int32(env, c->fragments[x].fragmentData.effectFragmentData().effectID, &fragprop);
      REJECT_STATUS;
      c->status =  napi_set_named_property(env, frag, "effectID", fragprop);
      REJECT_STATUS;
      break;
    default:
      break;
    }

    c->status =  napi_set_element(env, prop, x, frag);
    REJECT_STATUS;
  }
  c->status =  napi_set_named_property(env, result, "fragments", prop);
  REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value getFragments(napi_env env, napi_callback_info info) {
  getFragmentsCarrier* c = new getFragmentsCarrier;
  napi_value promise, prop, options, frag, resourceName;
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
		REJECT_ERROR_RETURN("Get Fragments must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

  if (argc < 2) {
    REJECT_ERROR_RETURN("Options object with clip ID and optional start and end must be provided.",
			QGW_INVALID_ARGS);
  }
  c->status = napi_typeof(env, argv[1], &type);
  REJECT_RETURN;
  c->status = napi_is_array(env, argv[1], &isArray);
  REJECT_RETURN;
  if (isArray || type != napi_object) {
    REJECT_ERROR_RETURN("Argument must be an options object with a clip ID and options start/end.",
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

// TODO leaving sync for now until requirements are clearer
napi_value cloneIfNeeded(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value prop, result;
	napi_valuetype type;
	bool isArray;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	int32_t clipID, poolID;
	bool highPriority = false;

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;
		CORBA::Boolean copyCreated;

		size_t argc = 2;
		napi_value argv[2];
		status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
		CHECK_STATUS;

		if (argc < 2) {
			NAPI_THROW_ORB_DESTROY("Options object with clip ID and pool ID must be provided.");
		}
		status = napi_typeof(env, argv[1], &type);
		CHECK_STATUS;
		status = napi_is_array(env, argv[1], &isArray);
		CHECK_STATUS;
		if (isArray || type != napi_object) {
			NAPI_THROW_ORB_DESTROY("Argument must be an options object with clip ID and pool ID.");
		}

		status = napi_get_named_property(env, argv[1], "clipID", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &clipID);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "poolID", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &poolID);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "highPriority", &prop);
		CHECK_STATUS;
		status = napi_typeof(env, prop, &type);
		CHECK_STATUS;
		if (type == napi_boolean) {
			status = napi_get_value_bool(env, prop, &highPriority);
			CHECK_STATUS;
		}

		zp->cloneIfNeeded(clipID, poolID, 0,
			highPriority ? Quentin::Port::HighPriority : Quentin::Port::StandardPriority,
			-1, copyCreated); // TODO -1 means the cloned clip never expires. Is this OK?

		status = napi_get_boolean(env, copyCreated, &result);
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