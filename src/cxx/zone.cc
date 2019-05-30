#include "zone.h"

void testConnectionExecute(napi_env env, void* data) {
	testConnectionCarrier* c = (testConnectionCarrier*) data;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);

		c->zoneNumber = zp->getZoneNumber();
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

void testConnectionComplete(napi_env env, napi_status asyncStatus, void* data) {
  testConnectionCarrier* c = (testConnectionCarrier*) data;
	napi_value result;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "Test connection failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_string_utf8(env, "PONG!", NAPI_AUTO_LENGTH, &result);
	REJECT_STATUS;

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value testConnection(napi_env env, napi_callback_info info) {
  testConnectionCarrier* c = new testConnectionCarrier;
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
		REJECT_ERROR_RETURN("Connection test must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	c->status = napi_create_string_utf8(env, "TestConnection", NAPI_AUTO_LENGTH, &resourceName);
  REJECT_RETURN;
  c->status = napi_create_async_work(env, nullptr, resourceName, testConnectionExecute,
    testConnectionComplete, c, &c->_request);
  REJECT_RETURN;
  c->status = napi_queue_async_work(env, c->_request);
  REJECT_RETURN;

  return promise;
}

void listZonesExecute(napi_env env, void* data) {
	listZonesCarrier* c = (listZonesCarrier*) data;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;

	try {
		resolveZonePortal(c->isaIOR, &orb, &zp);

		c->zoneIDs = zp->getZones(false);
		c->zoneIDs->length(c->zoneIDs->length() + 1);
		for ( int x = c->zoneIDs->length() - 1 ; x > 0 ; x-- ) {
			c->zoneIDs[x] = c->zoneIDs[x - 1];
		}
		c->zoneIDs[0] = zp->getZoneNumber(); // Always start with the local/default zone

		c->zoneNames = (CORBA::WChar**) malloc(c->zoneIDs->length() * sizeof(CORBA::WChar*));
		c->remotes = (CORBA::Boolean*) malloc(c->zoneIDs->length() * sizeof(CORBA::Boolean));

		for ( uint32_t x = 0 ; x < c->zoneIDs->length() ; x++ ) {
			c->zoneNames[x] = zp->getZoneName(c->zoneIDs[x]);
			c->remotes[x] = zp->zoneIsRemote(c->zoneIDs[x]);
		}
	}
	catch(CORBA::SystemException& ex) {
		NAPI_REJECT_CORBA_EXCEPTION(ex);
	}
	catch(CORBA::Exception& ex) {
		NAPI_REJECT_CORBA_EXCEPTION(ex);
	}
	catch(omniORB::fatalException& fe) {
		NAPI_REJECT_FATAL_EXCEPTION(fe);
	}

	orb->destroy();
}

void listZonesComplete(napi_env env, napi_status asyncStatus, void* data) {
	listZonesCarrier* c = (listZonesCarrier*) data;
	napi_value result, item, prop;
	CORBA::WChar* zoneName;
	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;

	if (asyncStatus != napi_ok) {
		c->status = asyncStatus;
		c->errorMsg = "List zones failed to complete.";
	}
	REJECT_STATUS;

	c->status = napi_create_array(env, &result);
	REJECT_STATUS;

	for ( uint32_t x = 0 ; x < c->zoneIDs->length() ; x++ ) {
		c->status = napi_create_object(env, &item);
		REJECT_STATUS;

		zoneName = c->zoneNames[x];

		c->status = napi_create_string_utf8(env, "ZonePortal", NAPI_AUTO_LENGTH, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, item, "type", prop);
		REJECT_STATUS;

		c->status = napi_create_int32(env, c->zoneIDs[x], &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, item, "zoneNumber", prop);
		REJECT_STATUS;

		std::string zoneNameStr = utf8_conv.to_bytes(zoneName);

		c->status = napi_create_string_utf8(env, zoneNameStr.c_str(), NAPI_AUTO_LENGTH, &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, item, "zoneName", prop);
		REJECT_STATUS;

		c->status = napi_get_boolean(env, c->remotes[x], &prop);
		REJECT_STATUS;
		c->status = napi_set_named_property(env, item, "isRemote", prop);
		REJECT_STATUS;

		c->status = napi_set_element(env, result, x, item);
		REJECT_STATUS;
	}

	napi_status status;
	status = napi_resolve_deferred(env, c->_deferred, result);
	FLOATING_STATUS;

	tidyCarrier(env, c);
}

napi_value listZones(napi_env env, napi_callback_info info) {
	listZonesCarrier* c = new listZonesCarrier;
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
		REJECT_ERROR_RETURN("Connection test must be provided with a IOR reference to an ISA server.",
	    QGW_INVALID_ARGS);
	}
	c->status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &iorLen);
	REJECT_RETURN;
	isaIOR = (char*) malloc((iorLen + 1) * sizeof(char));
	c->status = napi_get_value_string_utf8(env, argv[0], isaIOR, iorLen + 1, &iorLen);
	REJECT_RETURN;

	c->isaIOR = isaIOR;

	c->status = napi_create_string_utf8(env, "ListZones", NAPI_AUTO_LENGTH, &resourceName);
	REJECT_RETURN;
	c->status = napi_create_async_work(env, nullptr, resourceName, listZonesExecute,
		listZonesComplete, c, &c->_request);
	REJECT_RETURN;
	c->status = napi_queue_async_work(env, c->_request);
	REJECT_RETURN;

	return promise;
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
	  for ( uint32_t x = 0 ; x < serverIDs->length() ; x++ ) {
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
	      for ( uint32_t y = 0 ; y < serverInfo->pools.length() ; y++ ) {
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
	      for ( uint32_t y = 0 ; y < portNames->length(); y++ ) {
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
				for ( uint32_t y = 0 ; y < chanPorts->length() ; y++ ) {
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
