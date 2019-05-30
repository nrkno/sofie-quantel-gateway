#include "clip.h"

napi_value getClipData(napi_env env, napi_callback_info info) {
	napi_status status;
  napi_value result, prop, options;
  napi_valuetype type;
  bool isArray;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  int32_t clipID;

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
		status = napi_create_string_utf8(env, "ClipData", NAPI_AUTO_LENGTH, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "type", prop);
		CHECK_STATUS;

		options = argv[1];
		status = napi_get_named_property(env, options, "clipID", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &clipID);
		CHECK_STATUS;

		Quentin::ColumnDescList_var cdl = zp->getColumnDescriptions();
		Quentin::WStrings columns;
		columns.length(cdl->length());
		for ( uint32_t x = 0 ; x < cdl->length() ; x++ ) {
			columns[x] = cdl[x].columnName;
		}

		Quentin::WStrings_var results = zp->getClipData(clipID, columns);

		std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

		for ( uint32_t x = 0 ; x < results->length() ; x++ ) {
			std::string value = utf8_conv.to_bytes(results[x]);
			status = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &prop);
			CHECK_STATUS;

			if (wcscmp(cdl[x].columnType, L"Boolean") == 0) {
				status = napi_get_boolean(env, wcscmp(results[x], L"1") == 0, &prop);
				CHECK_STATUS;
			}

			if (wcscmp(cdl[x].columnType, L"Number") == 0) {
				if (value.length() > 0) {
					status = napi_create_int32(env, std::stol(value), &prop);
					CHECK_STATUS;
				} else {
					status = napi_get_null(env, &prop);
					CHECK_STATUS;
				}
			};

			if (wcscmp(cdl[x].columnType, L"Date") == 0) {
				if (value.length() > 0) {
					status = convertToDate(env, orb, value, &prop);
					CHECK_STATUS;
				} else {
					status = napi_get_null(env, &prop);
					CHECK_STATUS;
				}
			}



			std::string key = utf8_conv.to_bytes(columns[x]);
			status = napi_set_named_property(env, result, key.c_str(), prop);
			CHECK_STATUS;
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
				status = convertToDate(env, orb, value, &prop);
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
