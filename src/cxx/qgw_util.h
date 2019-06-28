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

#ifndef QGW_UTIL
#define QGW_UTIL

#include <Quentin.hh>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <cwchar>
#include <string>
#include <cstring>
#include <codecvt>
#include <locale>
#include <exception>
#include <inttypes.h>
#include "node_api.h"

// Now setting from binding.gyp
// #define OMNI_UNLOADABLE_STUBS

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }
#define CHECK_STATUS if (checkStatus(env, status, __FILE__, __LINE__ - 1) != napi_ok) { return nullptr; }
#define PASS_STATUS if (status != napi_ok) return status

napi_status checkStatus(napi_env env, napi_status status,
  const char* file, uint32_t line);

// Async error handling
#define QGW_ERROR_START 6000
#define QGW_INVALID_ARGS 6001
#define QGW_SYSTEM_EXCEPTION 6002
#define QGW_CORBA_EXCEPTION 6003
#define QGW_FATAL_EXCEPTION 6004
#define QGW_GET_PORT_FAIL 6005
#define QGW_SET_MODE_FAIL 6006
#define QGW_TRIGGER_SETUP_FAIL 6007
#define QGW_TRIGGER_OFFSET_FAIL 6008
#define QGW_TRIGGER_NOW_FAIL 6009
#define QGW_SUCCESS 0

#define NAPI_THROW_ERROR(msg) { \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

// TODO remove this ... no longer required
#define NAPI_THROW_ORB_DESTROY(msg) { \
  char errorMsg[256]; \
  snprintf(errorMsg, 256, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_SYSTEM_EXCEPTION(ex) { \
  char errorMsg[256]; \
  snprintf(errorMsg, 256, "System exception thrown from CORBA subsystem: %s.", ex._name()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

//snprintf(errorMsg, 256, "System exception thrown from CORBA subsystem with code 0x%lx: %s.", ex.minor(), ex._name());
#define NAPI_REJECT_SYSTEM_EXCEPTION(ex) { \
  char errorMsg[256]; \
	snprintf(errorMsg, 256, "System exception thrown from CORBA subsystem: %s.", ex._name()); \
	c->errorMsg = std::string(errorMsg); \
	c->status = QGW_SYSTEM_EXCEPTION; \
	return; \
}

#define NAPI_THROW_CORBA_EXCEPTION(ex) { \
  char errorMsg[256]; \
  snprintf(errorMsg, 256, "Exception thrown from CORBA subsystem: %s.", ex._name()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_REJECT_CORBA_EXCEPTION(ex) { \
	char errorMsg[256]; \
	snprintf(errorMsg, 256, "Exception thrown from CORBA subsystem: %s.", ex._name()); \
	c->errorMsg = std::string(errorMsg); \
	c->status = QGW_CORBA_EXCEPTION; \
	return; \
}

#define NAPI_THROW_FATAL_EXCEPTION(fe) { \
  char errorMsg[512]; \
  snprintf(errorMsg, 512, "Omni ORB fatal exception thrown at line %i of %s: %s.", fe.line(), fe.file(), fe.errmsg()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_REJECT_FATAL_EXCEPTION(fe) { \
	char errorMsg[512]; \
	snprintf(errorMsg, 512, "Omni ORB fatal exception thrown at line %i of %s: %s.", fe.line(), fe.file(), fe.errmsg()); \
	c->errorMsg = std::string(errorMsg); \
	c->status = QGW_FATAL_EXCEPTION; \
	return; \
}

#define START 0
#define STOP 1
#define JUMP 2
#define TRANSITION 3

struct carrier {
  napi_ref passthru = nullptr;
  int32_t status = QGW_SUCCESS;
  std::string errorMsg;
  long long totalTime;
  napi_deferred _deferred;
  napi_async_work _request = nullptr;
	char* isaIOR = nullptr;
	virtual ~carrier() {
		if (isaIOR != nullptr) {
			free(isaIOR);
		}
	}
};

void tidyCarrier(napi_env env, carrier* c);
int32_t rejectStatus(napi_env env, carrier* c, char* file, int32_t line);

#define REJECT_STATUS if (rejectStatus(env, c, (char*) __FILE__, __LINE__) != QGW_SUCCESS) return;
#define REJECT_BAIL if (rejectStatus(env, c, (char*) __FILE__, __LINE__) != QGW_SUCCESS) goto bail;
#define REJECT_RETURN if (rejectStatus(env, c, (char*) __FILE__, __LINE__) != QGW_SUCCESS) return promise;
#define FLOATING_STATUS if (status != napi_ok) { \
  printf("Unexpected N-API status not OK in file %s at line %d value %i.\n", \
    __FILE__, __LINE__ - 1, status); \
}

#define REJECT_ERROR(msg, status) { \
  c->errorMsg = msg; \
  c->status = status; \
  REJECT_STATUS; \
}

#define REJECT_ERROR_RETURN(msg, stat) { \
  c->errorMsg = msg; \
  c->status = stat; \
  REJECT_RETURN; \
}

napi_status retrieveZonePortal(napi_env env, napi_callback_info info,
	CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp);
napi_status resolveZonePortal(char* ior, CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp);
napi_status resolveZonePortalShared(char* ior, Quentin::ZonePortal::_ptr_type *zp);

std::string formatTimecode(Quentin::Timecode tc);
Quentin::Timecode timecodeFromString(std::string tcs);
napi_status convertToDate(napi_env env, std::string date, napi_value *nodeDate);

napi_status fragmentsToJS(napi_env env, Quentin::ServerFragments_var fragments, napi_value* prop);

napi_value destroyOrb(napi_env env, napi_callback_info info);

#endif // QGW_UTIL
