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

#include "qgw_util.h"

napi_status checkStatus(napi_env env, napi_status status,
  const char* file, uint32_t line) {

  napi_status infoStatus, throwStatus;
  const napi_extended_error_info *errorInfo;

  if (status == napi_ok) {
    // printf("Received status OK.\n");
    return status;
  }

  infoStatus = napi_get_last_error_info(env, &errorInfo);
  assert(infoStatus == napi_ok);
  printf("NAPI error in file %s on line %i. Error %i: %s\n", file, line,
    errorInfo->error_code, errorInfo->error_message);

  if (status == napi_pending_exception) {
    printf("NAPI pending exception. Engine error code: %i\n", errorInfo->engine_error_code);
    return status;
  }

  char errorCode[20];
  sprintf(errorCode, "%d", errorInfo->error_code);
  throwStatus = napi_throw_error(env, errorCode, errorInfo->error_message);
  assert(throwStatus == napi_ok);

  return napi_pending_exception; // Expect to be cast to void
}

napi_status retrieveZonePortal(napi_env env, napi_callback_info info, CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp) {
  napi_status status;
  char* isaIOR = nullptr;
  size_t iorLen = 0;

  size_t argc = 1;
  napi_value argv[1];
  status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  PASS_STATUS;

  if (argc < 1) {
    printf("Connection test must be provided with a IOR reference to an ISA server.");
    return napi_string_expected;
  }
  status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
  PASS_STATUS;
  isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
  status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
  PASS_STATUS;

  const char* options[][2] = { { "traceLevel", "1" }, { 0, 0 } };
  int orbc = 0;
  CORBA::ORB_var local_orb = CORBA::ORB_init(orbc, nullptr, "omniORB4", options);

  CORBA::Object_ptr ptr = local_orb->string_to_object(isaIOR);
  free(isaIOR);
	*zp = Quentin::ZonePortal::_unchecked_narrow(ptr);
  *orb = local_orb;

	return napi_ok;
}

napi_status resolveZonePortal(char* ior, CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp) {
  const char* options[][2] = { { "traceLevel", "1" }, { 0, 0 } };
  int orbc = 0;
  CORBA::ORB_var local_orb = CORBA::ORB_init(orbc, nullptr, "omniORB4", options);

  CORBA::Object_ptr ptr = local_orb->string_to_object(ior);
	*zp = Quentin::ZonePortal::_unchecked_narrow(ptr);
  *orb = local_orb;

	return napi_ok;
}

std::string formatTimecode(Quentin::Timecode tc) {
	int32_t HH, hh, MM, mm, SS, ss, FF, ff;
	bool drop = (tc & 0x40000000) != 0;
	char* tcstr = (char*) malloc(12 * sizeof(char));
	HH = (tc >> 28) & 0x03;
	hh = (tc >> 24) & 0x0f;
	MM = (tc >> 20) & 0x07;
	mm = (tc >> 16) & 0x0f;
	SS = (tc >> 12) & 0x07;
	ss = (tc >>  8) & 0x0f;
	FF = (tc >>  4) & 0x07;
	ff = (tc >>  0) & 0x0f;
	snprintf(tcstr, 12, "%01x%01x:%01x%01x:%01x%01x%s%01x%01x", HH, hh, MM, mm, SS, ss,
    drop ? ";" : ":", FF, ff);
	std::string result(tcstr);
	free(tcstr);
	return result;
}

Quentin::Timecode timecodeFromString(std::string tcs) {
	int32_t HH, hh, MM, mm, SS, ss, FF, ff;
	bool drop;
	HH = std::stoi(tcs.substr(0, 1));
	hh = std::stoi(tcs.substr(1, 1));
	MM = std::stoi(tcs.substr(3, 1));
	mm = std::stoi(tcs.substr(4, 1));
	SS = std::stoi(tcs.substr(6, 1));
	ss = std::stoi(tcs.substr(7, 1));
	FF = std::stoi(tcs.substr(9, 1));
	ff = std::stoi(tcs.substr(10, 1));
	drop = tcs.substr(8, 1) == ";";
	Quentin::Timecode tc =
		HH << 28 | hh << 24 |
		MM << 20 | mm << 16 |
		SS << 12 | ss <<  8 |
		FF <<  4 | ff;
	if (drop) tc = tc | 0x40000000;
	return tc;
}

napi_status convertToDate(napi_env env, std::string date, napi_value *nodeDate) {
	napi_status status;
	napi_value global, dateObj, integerDate;

	status = napi_get_global(env, &global);
	PASS_STATUS;

	status = napi_get_named_property(env, global, "Date", &dateObj);
	PASS_STATUS;
	status = napi_create_int64(env, std::stoll(date), &integerDate);
	PASS_STATUS;

	napi_value argv[1] = { integerDate };
	status = napi_new_instance(env, dateObj, 1, argv, nodeDate);
	PASS_STATUS;

	return napi_ok;
}

void tidyCarrier(napi_env env, carrier* c) {
  napi_status status;
  if (c->passthru != nullptr) {
    status = napi_delete_reference(env, c->passthru);
    FLOATING_STATUS;
  }
  if (c->_request != nullptr) {
    status = napi_delete_async_work(env, c->_request);
    FLOATING_STATUS;
  }
  // printf("Tidying carrier %p %p\n", c->passthru, c->_request);
  delete c;
}

int32_t rejectStatus(napi_env env, carrier* c, char* file, int32_t line) {
  if (c->status != QGW_SUCCESS) {
    napi_value errorValue, errorCode, errorMsg;
    napi_status status;
    if (c->status < QGW_ERROR_START) {
      const napi_extended_error_info *errorInfo;
      status = napi_get_last_error_info(env, &errorInfo);
      FLOATING_STATUS;
      c->errorMsg = std::string(
        (errorInfo->error_message != nullptr) ? errorInfo->error_message : "(no message)");
    }
    char* extMsg = (char *) malloc(sizeof(char) * c->errorMsg.length() + 200);
    sprintf(extMsg, "In file %s on line %i, found error: %s", file, line, c->errorMsg.c_str());
    char errorCodeChars[20];
    sprintf(errorCodeChars, "%d", c->status);
    status = napi_create_string_utf8(env, errorCodeChars,
      NAPI_AUTO_LENGTH, &errorCode);
    FLOATING_STATUS;
    status = napi_create_string_utf8(env, extMsg, NAPI_AUTO_LENGTH, &errorMsg);
    FLOATING_STATUS;
    status = napi_create_error(env, errorCode, errorMsg, &errorValue);
    FLOATING_STATUS;
    status = napi_reject_deferred(env, c->_deferred, errorValue);
    FLOATING_STATUS;

    delete[] extMsg;
    tidyCarrier(env, c);
  }
  return c->status;
}

napi_status fragmentsToJS(napi_env env, Quentin::ServerFragments_var fragments, napi_value* prop) {
	napi_status status;
	napi_value frag, fragprop;
	char rushID[33];
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	status = napi_create_array(env, prop);
	PASS_STATUS;

	for ( uint32_t x = 0 ; x < fragments->length() ; x++ ) {
		status = napi_create_object(env, &frag);
		PASS_STATUS;

		switch (fragments[x].fragmentData._d()) {
		case Quentin::FragmentType::videoFragment:
			status = napi_create_string_utf8(env, "VideoFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::audioFragment:
			status = napi_create_string_utf8(env, "AudioFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::auxFragment:
			status = napi_create_string_utf8(env, "AUXFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::flagsFragment:
			status = napi_create_string_utf8(env, "FlagsFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::timecodeFragment:
			status = napi_create_string_utf8(env, "TimecodeFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::aspectFragment:
			status = napi_create_string_utf8(env, "AspectFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::cropFragment:
			status = napi_create_string_utf8(env, "CropFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::panZoomFragment:
			status = napi_create_string_utf8(env, "PanZoomFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::speedFragment:
			status = napi_create_string_utf8(env, "SpeedFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::multiCamFragment:
			status = napi_create_string_utf8(env, "MultiCamFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::ccFragment:
			status = napi_create_string_utf8(env, "CCFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::noteFragment:
			status = napi_create_string_utf8(env, "NoteFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::effectFragment:
			status = napi_create_string_utf8(env, "EffectFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		default:
			status = napi_create_string_utf8(env, "ServerFragment", NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			break;
		}
		status = napi_set_named_property(env, frag, "type", fragprop);
		PASS_STATUS;

		status = napi_create_int32(env, fragments[x].trackNum, &fragprop);
		PASS_STATUS;
		status = napi_set_named_property(env, frag, "trackNum", fragprop);
		PASS_STATUS;

		status = napi_create_int32(env, fragments[x].start, &fragprop);
		PASS_STATUS;
		status = napi_set_named_property(env, frag, "start", fragprop);
		PASS_STATUS;

		status = napi_create_int32(env, fragments[x].finish, &fragprop);
		PASS_STATUS;
		status = napi_set_named_property(env, frag, "finish", fragprop);
		PASS_STATUS;

		switch (fragments[x].fragmentData._d()) {
		case Quentin::FragmentType::videoFragment:
			sprintf(rushID, "%016" PRIx64 "%016" PRIx64,// "%016llx%016llx",
				fragments[x].fragmentData.videoFragmentData().rushID.first,
				fragments[x].fragmentData.videoFragmentData().rushID.second);
			status = napi_create_string_utf8(env, rushID, 32, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "rushID", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().format, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "format", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().poolID, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "poolID", fragprop);
			PASS_STATUS;

			status = napi_create_int64(env, fragments[x].fragmentData.videoFragmentData().poolFrame, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "poolFrame", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().skew, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "skew", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.videoFragmentData().rushFrame, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "rushFrame", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::audioFragment:
			sprintf(rushID, "%016" PRIx64 "%016" PRIx64,// "%016llx%016llx",
				fragments[x].fragmentData.audioFragmentData().rushID.first,
				fragments[x].fragmentData.audioFragmentData().rushID.second);
			status = napi_create_string_utf8(env, rushID, 32, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "rushID", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().format, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "format", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().poolID, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "poolID", fragprop);
			PASS_STATUS;

			status = napi_create_int64(env, fragments[x].fragmentData.audioFragmentData().poolFrame, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "poolFrame", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().skew, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "skew", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.audioFragmentData().rushFrame, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "rushFrame", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::auxFragment:
			sprintf(rushID, "%016" PRIx64 "%016" PRIx64, // "%016llx%016llx",
				fragments[x].fragmentData.auxFragmentData().rushID.first,
				fragments[x].fragmentData.auxFragmentData().rushID.second);
			status = napi_create_string_utf8(env, rushID, 32, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "rushID", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().format, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "format", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().poolID, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "poolID", fragprop);
			PASS_STATUS;

			status = napi_create_int64(env, fragments[x].fragmentData.auxFragmentData().poolFrame, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "poolFrame", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().skew, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "skew", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.auxFragmentData().rushFrame, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "rushFrame", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::flagsFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.flagsFragmentData().flags, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "flags", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::timecodeFragment:
			status = napi_create_string_utf8(env,
				formatTimecode(fragments[x].fragmentData.timecodeFragmentData().startTimecode).c_str(),
				NAPI_AUTO_LENGTH, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "startTimecode", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.timecodeFragmentData().userBits, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "userBits", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::aspectFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.aspectFragmentData().width, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "width", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.aspectFragmentData().height, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "height", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::cropFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.cropFragmentData().x, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "x", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.cropFragmentData().y, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "y", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.cropFragmentData().width, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "width", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.cropFragmentData().height, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "height", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::noteFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.noteFragmentData().noteID, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "noteID", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.noteFragmentData().aux, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "aux", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.noteFragmentData().mask, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "mask", fragprop);
			PASS_STATUS;

			if (fragments[x].fragmentData.noteFragmentData().note == nullptr) {
				status = napi_get_null(env, &fragprop);
				PASS_STATUS;
			} else {
				status = napi_create_string_utf8(env,
					utf8_conv.to_bytes(std::wstring(fragments[x].fragmentData.noteFragmentData().note)).c_str(),
					NAPI_AUTO_LENGTH, &fragprop);
				PASS_STATUS;
			}
			status = napi_set_named_property(env, frag, "note", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::panZoomFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.panZoomFragmentData().x, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "x", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.panZoomFragmentData().y, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "y", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.panZoomFragmentData().hZoom, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "hZoom", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.panZoomFragmentData().vZoom, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "vZoom", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::speedFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.speedFragmentData().speed, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "speed", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.speedFragmentData().profile, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "profile", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::multiCamFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.multiCamFragmentData().stream, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "stream", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::ccFragment:
			sprintf(rushID, "%016" PRIx64 "%016" PRIx64, // "%016llx%016llx",
				fragments[x].fragmentData.ccFragmentData().ccID.first,
				fragments[x].fragmentData.ccFragmentData().ccID.second);
			status = napi_create_string_utf8(env, rushID, 32, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "ccID", fragprop);
			PASS_STATUS;

			status = napi_create_int32(env, fragments[x].fragmentData.ccFragmentData().ccType, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "ccType", fragprop);
			PASS_STATUS;
			break;
		case Quentin::FragmentType::effectFragment:
			status = napi_create_int32(env, fragments[x].fragmentData.effectFragmentData().effectID, &fragprop);
			PASS_STATUS;
			status = napi_set_named_property(env, frag, "effectID", fragprop);
			PASS_STATUS;
			break;
		default:
			break;
		}

		status = napi_set_element(env, *prop, x, frag);
		PASS_STATUS;
	}

	return napi_ok;
}
