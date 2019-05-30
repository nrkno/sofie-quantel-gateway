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
#include "node_api.h"

// Now setting from binding.gyp
// #define OMNI_UNLOADABLE_STUBS

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }
#define CHECK_STATUS if (checkStatus(env, status, __FILE__, __LINE__ - 1) != napi_ok) { orb->destroy(); return nullptr; }
#define PASS_STATUS if (status != napi_ok) return status

napi_status checkStatus(napi_env env, napi_status status,
  const char* file, uint32_t line);

#define NAPI_THROW_ERROR(msg) { \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_ORB_DESTROY(msg) { \
  orb->destroy(); \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_SYSTEM_EXCEPTION(ex) { \
  orb->destroy(); \
  char errorMsg[256]; \
  sprintf(errorMsg, "System exception thrown from CORBA subsystem with code %lu: %s", ex.minor(), ex._name()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_CORBA_EXCEPTION(ex) { \
  orb->destroy(); \
  char errorMsg[256]; \
  sprintf(errorMsg, "Exception thrown from CORBA subsystem: %s", ex._name()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define NAPI_THROW_FATAL_EXCEPTION(fe) { \
  orb->destroy(); \
  char errorMsg[512]; \
  sprintf(errorMsg, "Omni ORB fatal exception thrown at line %i of %s: %s", fe.line(), fe.file(), fe.errmsg()); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

#define START 0
#define STOP 1
#define JUMP 2
#define TRANSITION 3

napi_status retrieveZonePortal(napi_env env, napi_callback_info info,
	CORBA::ORB_var *orb, Quentin::ZonePortal::_ptr_type *zp);
char* formatTimecode(Quentin::Timecode tc);
napi_status convertToDate(napi_env env, CORBA::ORB_var orb, std::string date, napi_value *nodeDate);

#endif // QGW_UTIL
