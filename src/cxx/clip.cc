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


napi_value searchClips(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value result, prop, options, terms, term;
	napi_valuetype type;
	bool isArray;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::ClipPropertyList cpl;
	uint32_t termCount, resultCount;
	char* nameStr;
	char* valueStr;
	size_t strLen;

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;

		size_t argc = 2;
		napi_value argv[2];
		status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
		CHECK_STATUS;

		if (argc < 2) {
			NAPI_THROW_ORB_DESTROY("Options object with search terms must be provided.");
		}
		status = napi_typeof(env, argv[1], &type);
		CHECK_STATUS;
		status = napi_is_array(env, argv[1], &isArray);
		CHECK_STATUS;
		if (isArray || type != napi_object) {
			NAPI_THROW_ORB_DESTROY("Argument must be an options object with search terms.");
		}

		status = napi_create_array(env, &result);
		CHECK_STATUS;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

		options = argv[1];
		status = napi_get_property_names(env, options, &terms);
		CHECK_STATUS;
		status = napi_get_array_length(env, terms, &termCount);
		CHECK_STATUS;
		cpl.length(termCount);
		for ( uint32_t x = 0 ; x < termCount ; x++ ) {
			status = napi_get_element(env, terms, x, &term);
			CHECK_STATUS;
			status = napi_get_property(env, options, term, &prop);
			CHECK_STATUS;

			status = napi_get_value_string_utf8(env, term, nullptr, 0, &strLen);
			CHECK_STATUS;
			nameStr = (char *) malloc((strLen + 1) * sizeof(char));
			status = napi_get_value_string_utf8(env, term, nameStr, strLen + 1, &strLen);
			CHECK_STATUS;

			status = napi_get_value_string_utf8(env, prop, nullptr, 0, &strLen);
			CHECK_STATUS;
			valueStr = (char *) malloc((strLen + 1) * sizeof(char));
			status = napi_get_value_string_utf8(env, prop, valueStr, strLen + 1, &strLen);
			CHECK_STATUS;

			Quentin::ClipProperty cp = {
				utf8_conv.from_bytes(nameStr).data(),
				utf8_conv.from_bytes(valueStr).data() };
			cpl[x] = cp;
			// wprintf(L"%ws = %ws\n", cp.name, cp.value);
			free(nameStr);
			free(valueStr);
		}

		Quentin::WStrings columns;
		columns.length(7);
		columns[0] = L"ClipID";
		columns[1] = L"Completed";
		columns[2] = L"Created";
		columns[3] = L"Description";
		columns[4] = L"Frames";
		columns[5] = L"Owner";
		columns[6] = L"Title";

		status = napi_create_object(env, &term);
		CHECK_STATUS;
		status = napi_create_string_utf8(env, "ClipDataSummary", NAPI_AUTO_LENGTH, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, term, "type", prop);
		CHECK_STATUS;

		Quentin::WStrings_var results = zp->searchClips(cpl, columns, 10);
		resultCount = 0;
		for ( uint32_t x = 0 ; x < results->length() ; x++ ) {
			std::string value = utf8_conv.to_bytes(results[x]);
			std::string key = utf8_conv.to_bytes(columns[x % 7]);

			if (strcmp(key.c_str(), "ClipID") == 0) {
				status = napi_create_int32(env, std::stol(value), &prop);
				CHECK_STATUS;
			} else if (strcmp(key.c_str(), "Completed") == 0 || strcmp(key.c_str(), "Created") == 0) {
				status = convertToDate(env, value, &prop);
				CHECK_STATUS;
			} else {
				status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &prop);
				CHECK_STATUS;
			}
			status = napi_set_named_property(env, term, key.c_str(), prop);
			CHECK_STATUS;

			if (x % 7 == 6) {
				status = napi_set_element(env, result, resultCount++, term);
				CHECK_STATUS;
				if (x < results->length() - 2) {
					status = napi_create_object(env, &term);
					CHECK_STATUS;
					status = napi_create_string_utf8(env, "ClipDataSummary", NAPI_AUTO_LENGTH, &prop);
					CHECK_STATUS;
					status = napi_set_named_property(env, term, "type", prop);
					CHECK_STATUS;
				}
			}
		}

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

napi_value getFragments(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, options, frag, fragprop;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t clipID;
  char rushID[33];
	int32_t start = -1, finish = -1;

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

		status = napi_get_named_property(env, options, "start", &prop);
		CHECK_STATUS;
		status = napi_typeof(env, prop, &type);
		CHECK_STATUS;
		if (type == napi_number) {
			status = napi_get_value_int32(env, prop, &start);
			CHECK_STATUS;
		}

		status = napi_get_named_property(env, options, "finish", &prop);
		CHECK_STATUS;
		status = napi_typeof(env, prop, &type);
		CHECK_STATUS;
		if (type == napi_number) {
			status = napi_get_value_int32(env, prop, &finish);
			CHECK_STATUS;
		}

    Quentin::ServerFragments_var fragments =
		  (start >= 0) && (finish >= 0) ? zp->getFragments(clipID, start, finish) : zp->getAllFragments(clipID);

    status = napi_create_array(env, &prop);
    CHECK_STATUS;

    for ( uint32_t x = 0 ; x < fragments->length() ; x++ ) {
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
