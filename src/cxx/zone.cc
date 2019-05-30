#include "zone.h"

napi_value testConnection(napi_env env, napi_callback_info info) {

  napi_status status;
  napi_value result;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;

	try {
	  status = retrieveZonePortal(env, info, &orb, &zp);
	  CHECK_STATUS;

	  long zoneNumber = zp->getZoneNumber();
	  status = napi_get_boolean(env, zoneNumber > 0, &result);
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

napi_value listZones(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value result, item, prop;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	CORBA::WChar* zoneName;
	Quentin::Longs_var zoneIDs;

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;

		status = napi_create_array(env, &result);
		CHECK_STATUS;

		zoneIDs = zp->getZones(false);
		zoneIDs->length(zoneIDs->length() + 1);
		for ( int x = zoneIDs->length() - 1 ; x > 0 ; x-- ) {
			zoneIDs[x] = zoneIDs[x - 1];
		}
		zoneIDs[0] = zp->getZoneNumber(); // Always start with the local/default zone

		for ( int x = 0 ; x < zoneIDs->length() ; x++ ) {
			status = napi_create_object(env, &item);
			CHECK_STATUS;

			zoneName = zp->getZoneName(zoneIDs[x]);

			status = napi_create_string_utf8(env, "ZonePortal", NAPI_AUTO_LENGTH, &prop);
			CHECK_STATUS;
			status = napi_set_named_property(env, item, "type", prop);
			CHECK_STATUS;

			status = napi_create_int32(env, zoneIDs[x], &prop);
			CHECK_STATUS;
			status = napi_set_named_property(env, item, "zoneNumber", prop);
			CHECK_STATUS;

			std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
			std::string zoneNameStr = utf8_conv.to_bytes(zoneName);

			status = napi_create_string_utf8(env, zoneNameStr.c_str(), NAPI_AUTO_LENGTH, &prop);
			CHECK_STATUS;
			status = napi_set_named_property(env, item, "zoneName", prop);
			CHECK_STATUS;

			status = napi_get_boolean(env, zp->zoneIsRemote(zoneIDs[x]), &prop);
			CHECK_STATUS;
			status = napi_set_named_property(env, item, "isRemote", prop);
			CHECK_STATUS;

			status = napi_set_element(env, result, x, item);
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

napi_value getDefaultZoneInfo(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  CORBA::WChar* zoneName;

	try {
	  status = retrieveZonePortal(env, info, &orb, &zp);
	  CHECK_STATUS;

	  long zoneNumber = zp->getZoneNumber();
	  zoneName = zp->getZoneName(zoneNumber);

	  status = napi_create_object(env, &result);
	  CHECK_STATUS;

	  status = napi_create_string_utf8(env, "ZonePortal", NAPI_AUTO_LENGTH, &prop);
	  CHECK_STATUS;
	  status = napi_set_named_property(env, result, "type", prop);
	  CHECK_STATUS;

	  status = napi_create_int32(env, zoneNumber, &prop);
	  CHECK_STATUS;
	  status = napi_set_named_property(env, result, "zoneNumber", prop);
	  CHECK_STATUS;

	  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
	  std::string zoneNameStr = utf8_conv.to_bytes(zoneName);

	  status = napi_create_string_utf8(env, zoneNameStr.c_str(), NAPI_AUTO_LENGTH, &prop);
	  CHECK_STATUS;
	  status = napi_set_named_property(env, result, "zoneName", prop);
	  CHECK_STATUS;

		status = napi_get_boolean(env, false, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "isRemote", prop);
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

napi_value getServers(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value result, prop, subprop, item;
  CORBA::ORB_var orb;
  Quentin::ZonePortal::_ptr_type zp;
  Quentin::Server_ptr server;
  Quentin::ServerInfo* serverInfo;
  Quentin::WStrings_var portNames, chanPorts;
  std::string portName, serverNameStr;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	try {
	  status = retrieveZonePortal(env, info, &orb, &zp);
	  CHECK_STATUS;

	  status = napi_create_array(env, &result);
	  CHECK_STATUS;

	  Quentin::Longs_var serverIDs = zp->getServers(true);
	  for ( int x = 0 ; x < serverIDs->length() ; x++ ) {
	    status = napi_create_object(env, &item);
	    CHECK_STATUS;

	    status = napi_create_string_utf8(env, "Server", NAPI_AUTO_LENGTH, &prop);
	    CHECK_STATUS;
	    status = napi_set_named_property(env, item, "type", prop);
	    CHECK_STATUS;

	    status = napi_create_int64(env, serverIDs[x] > 0 ? serverIDs[x] : -serverIDs[x], &prop);
	    CHECK_STATUS;
	    status = napi_set_named_property(env, item, "ident", prop);
	    CHECK_STATUS;

	    if (serverIDs[x] > 0) { // server is o
	      server = zp->getServer(serverIDs[x]);
	      serverInfo = server->getServerInfo();

	      status = napi_get_boolean(env, serverInfo->down, &prop);
	      CHECK_STATUS;
	      status = napi_set_named_property(env, item, "down", prop);
	      CHECK_STATUS;

	      std::string serverNameStr = utf8_conv.to_bytes(serverInfo->name);

	      status = napi_create_string_utf8(env, serverNameStr.c_str(), NAPI_AUTO_LENGTH, &prop);
	      CHECK_STATUS;
	      status = napi_set_named_property(env, item, "name", prop);
	      CHECK_STATUS;

	      status = napi_create_int32(env, serverInfo->numChannels, &prop);
	      CHECK_STATUS;
	      status = napi_set_named_property(env, item, "numChannels", prop);
	      CHECK_STATUS;

	      status = napi_create_array(env, &prop);
	      CHECK_STATUS;
	      for ( int y = 0 ; y < serverInfo->pools.length() ; y++ ) {
	        status = napi_create_int32(env, serverInfo->pools[y], &subprop);
	        CHECK_STATUS;
	        status = napi_set_element(env, prop, y, subprop);
	        CHECK_STATUS;
	      }
	      status = napi_set_named_property(env, item, "pools", prop);
	      CHECK_STATUS;

	      portNames = server->getPortNames();
	      status = napi_create_array(env, &prop);
	      CHECK_STATUS;
	      for ( int y = 0 ; y < portNames->length(); y++ ) {
		      portName = utf8_conv.to_bytes(portNames[y]);

	        status = napi_create_string_utf8(env, portName.c_str(), NAPI_AUTO_LENGTH, &subprop);
	        CHECK_STATUS;
	        status = napi_set_element(env, prop, y, subprop);
	        CHECK_STATUS;
	      }
	      status = napi_set_named_property(env, item, "portNames", prop);
	      CHECK_STATUS;

				chanPorts = server->getChanPorts();
				status = napi_create_array(env, &prop);
				CHECK_STATUS;
				for ( int y = 0 ; y < chanPorts->length() ; y++ ) {
					portName = utf8_conv.to_bytes(chanPorts[y]);

					status = napi_create_string_utf8(env, portName.c_str(), NAPI_AUTO_LENGTH, &subprop);
					CHECK_STATUS;
					status = napi_set_element(env, prop, y, subprop);
					CHECK_STATUS;
				}
				status = napi_set_named_property(env, item, "chanPorts", prop);
				CHECK_STATUS;
	    } else {
	      status = napi_get_boolean(env, true, &prop);
	      CHECK_STATUS;
	      status = napi_set_named_property(env, item, "down", prop);
	      CHECK_STATUS;
	    }

	    status = napi_set_element(env, result, x, item);
	    CHECK_STATUS;
	  }
	}
	catch(CORBA::SystemException& ex) {
		printf("I am a system exception!!!, %i\n", omni::TRANSIENT_minor::TRANSIENT_ConnectFailed);
		NAPI_THROW_SYSTEM_EXCEPTION(ex);
	}
	catch(CORBA::Exception& ex) {
		printf("I am a minion system exception!!!\n");
		NAPI_THROW_CORBA_EXCEPTION(ex);
	}
	catch(omniORB::fatalException& fe) {
		NAPI_THROW_FATAL_EXCEPTION(fe);
	}

  orb->destroy();
  return result;
}
