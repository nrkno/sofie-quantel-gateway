#ifndef QGW_THUMBS
#define QGW_THUMBS

#include "qgw_util.h"

napi_value getThumbnailSize(napi_env env, napi_callback_info info);
napi_value requestThumbnails(napi_env env, napi_callback_info info);

class ThumbyListener : public POA_Quentin::ThumbnailListener {
public:
	inline ThumbyListener(CORBA::ORB_var orby /*, int32_t tnCount */) : ident(ID++) {
		orb = CORBA::ORB::_duplicate(orby);
		// count = tnCount;
	}
	virtual ~ThumbyListener() { free(lastFrame); }
	virtual void newThumbnail(CORBA::Long ident, CORBA::Long offset, CORBA::Long width, CORBA::Long height, const Quentin::Longs & data);
	virtual void noThumbnail(Quentin::ThumbnailListener::NoThumbnailReason reason, CORBA::Long ident, CORBA::Long offset, CORBA::Boolean tryAgainLater, const CORBA::WChar * reasonStr);
  virtual void finished(CORBA::Long ident);
	virtual char* getLastFrame();
	virtual size_t getLastFrameSize();
	virtual inline int32_t getIdent() { return ident; };
private:
	CORBA::ORB_ptr orb;
	size_t lastFrameSize = 0;
	char* lastFrame = nullptr;
	static int32_t ID;
	int ident;
};

#endif // QGW_THUMBS
