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

#include "thumbs.h"

int32_t ThumbyListener::ID = 0;

void ThumbyListener::newThumbnail(CORBA::Long ident, CORBA::Long offset, CORBA::Long width, CORBA::Long height, const Quentin::Longs & data) {
	printf("newThumbnail called - ident %i, offset %i, dimensions %ix%i, data length %i\n", ident, offset, width, height, data.length());
	if (lastFrame != nullptr) free(lastFrame);
	lastFrameSize = data.length() * 4;
	lastFrame = (char*) malloc(lastFrameSize * sizeof(char));
	for ( uint32_t x = 0 ; x < data.length() ; x++ ) {
		lastFrame[x * 4] = (data[x] >> 24) & 0xff;
		lastFrame[x * 4 + 1] = (data[x] >> 16) & 0xff;
		lastFrame[x * 4 + 2] = (data[x] >> 8) & 0xff;
		lastFrame[x * 4 + 3] = data[x] & 0xff;
	}
}

void ThumbyListener::noThumbnail(Quentin::ThumbnailListener::NoThumbnailReason reason, CORBA::Long ident, CORBA::Long offset, CORBA::Boolean tryAgainLater, const CORBA::WChar * reasonStr) {
	printf("noThumbnail called\n");
}

void ThumbyListener::finished(CORBA::Long ident) {
	printf("finished called\n");
	orb->shutdown(false);
	CORBA::release(orb);
}

char* ThumbyListener::getLastFrame() {
	return lastFrame;
}

size_t ThumbyListener::getLastFrameSize() {
	return lastFrameSize;
}

napi_value getThumbnailSize(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value prop, result;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	CORBA::Long width(0);
	CORBA::Long height(0);

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;

		zp->getThumbnailSize(0, width, height);

		status = napi_create_object(env, &result);
		CHECK_STATUS;

		status = napi_create_int32(env, width, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "width", prop);
		CHECK_STATUS;

		status = napi_create_int32(env, height, &prop);
		CHECK_STATUS;
		status = napi_set_named_property(env, result, "height", prop);
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

napi_value requestThumbnails(napi_env env, napi_callback_info info) {
	napi_status status;
	napi_value prop, result;
	napi_valuetype type;
	bool isArray;
	CORBA::ORB_var orb;
	Quentin::ZonePortal::_ptr_type zp;
	Quentin::ServerFragments_var fragments;
	int32_t clipID, offset, stride, tnCount;

	try {
		status = retrieveZonePortal(env, info, &orb, &zp);
		CHECK_STATUS;

		size_t argc = 2;
		napi_value argv[2];
		status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
		CHECK_STATUS;

		if (argc < 2) {
			NAPI_THROW_ORB_DESTROY("Options object with clip ID, offset, stride and thumbnail count must be provided.");
		}
		status = napi_typeof(env, argv[1], &type);
		CHECK_STATUS;
		status = napi_is_array(env, argv[1], &isArray);
		CHECK_STATUS;
		if (isArray || type != napi_object) {
			NAPI_THROW_ORB_DESTROY("Argument must be an options object with clip ID, offset, stride and thumbnail count.");
		}

		status = napi_get_named_property(env, argv[1], "clipID", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &clipID);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "offset", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &offset);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "stride", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &stride);
		CHECK_STATUS;

		status = napi_get_named_property(env, argv[1], "count", &prop);
		CHECK_STATUS;
		status = napi_get_value_int32(env, prop, &tnCount);
		CHECK_STATUS;

		fragments = zp->getTypeFragments(clipID, 0);

		CORBA::Object_var       obj = orb->resolve_initial_references("RootPOA");
		PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

		PortableServer::Servant_var<ThumbyListener> mythumby = new ThumbyListener(orb);
		PortableServer::ObjectId_var mythumbyid = poa->activate_object(mythumby);

		PortableServer::POAManager_var pman = poa->the_POAManager();
		pman->activate();

		Quentin::ThumbnailListener_ptr qtip = Quentin::ThumbnailListener::_duplicate(mythumby->_this());
		zp->requestThumbnails(0, fragments[0].fragmentData.videoFragmentData(), offset, stride, tnCount, mythumby->getIdent(), qtip);

		orb->run();
		CORBA::release(qtip);
		// poa->deactivate_object(mythumbyid);
		// pman->deactivate(true, true);
		void* resultData;

		status = napi_create_buffer_copy(env, mythumby->getLastFrameSize(), mythumby->getLastFrame(),
		  &resultData, &result);
		CHECK_STATUS;
		printf("End of try scope.\n");
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
	printf("Calling orb->destroy()\n");
	orb->destroy();
	printf("Reached return statement.\n");
	return result;
}
