#include "test_server.h"

class Server_i: public POA_Quentin::Server,
        public PortableServer::RefCountServantBase
{
public:
  inline Server_i() {}
  virtual ~Server_i() {}
	virtual Quentin::ServerInfo* getServerInfo() { return nullptr; }
	virtual Quentin::Port_ptr getPort(const ::CORBA::WChar* ident, ::CORBA::Long number) { return nullptr; }
	virtual Quentin::WStrings* getPortNames() { return nullptr; }
	virtual Quentin::WStrings* getChanPorts() { return nullptr; }
	virtual ::CORBA::LongLong getFreeProtons(::CORBA::Long poolIdent) { return 0; }
	virtual Quentin::Longs* getPools() { return nullptr; }
	virtual Quentin::Timecode getRefTime() { return Quentin::Timecode(); }
	virtual Quentin::ConfigDescriptionList* getConfigurations(::CORBA::Long channel, ::Quentin::FragmentType type, ::CORBA::Boolean forPlay) { return nullptr; }
	virtual Quentin::Longs* getDefaultConfigurations(::CORBA::Long channel) { return nullptr; }
	virtual Quentin::Longs* getCurrentConfigurations(::CORBA::Long channel) { return nullptr; }
	virtual Quentin::ServerCapabilities* getServerCapabilities() { return nullptr; }

	virtual ::CORBA::WChar* getProperty(const ::CORBA::WChar* propertyName) { return nullptr; }
	virtual Quentin::WStrings* getPropertyList() { return nullptr; }
};

class ZonePortal_i : public POA_Quentin::ZonePortal,
        public PortableServer::RefCountServantBase
{
public:
  inline ZonePortal_i() {}
  virtual ~ZonePortal_i() {}
	virtual ::CORBA::Long majorIDLVersion() { return 0; }
	virtual Quentin::Longs* getServers(::CORBA::Boolean negateIfDown);
	virtual Quentin::Server_ptr getServer(::CORBA::Long serverID);
	virtual Quentin::Longs* getPools() { return nullptr; }
	virtual ::CORBA::LongLong getPoolSpace(::CORBA::Long mode, ::CORBA::Long poolID) { return 0; }
	virtual Quentin::Server_ptr getPoolServer(::CORBA::Long poolID) { return nullptr; }
	virtual ::CORBA::LongLong getServerSpace(::CORBA::Long mode, ::CORBA::Long serverID) { return 0; }
	virtual Quentin::PoolInfo* getPoolInfo(::CORBA::Long poolID) { return nullptr; }
	virtual Quentin::DirectoryViewer_ptr getDirViewer(::CORBA::Long timeoutSecs, const ::CORBA::WChar* viewerName) { return nullptr; }
	virtual Quentin::DirectoryViewer_ptr getPoolDirViewer(::CORBA::Long poolID, ::CORBA::Long timeoutSecs, const ::CORBA::WChar* viewerName) { return nullptr; }
	virtual ::CORBA::Boolean addStateChangeListener(::Quentin::StateChangeListener_ptr listener, ::CORBA::Long flags) { return nullptr; }
	virtual ::CORBA::Boolean addNamedStateChangeListener(const ::CORBA::WChar* listenerName, ::Quentin::StateChangeListener_ptr listener, ::CORBA::Long flags, ::CORBA::Long interval) { return nullptr; }
	virtual ::CORBA::Boolean removeStateChangeListener(::Quentin::StateChangeListener_ptr listener) { return nullptr; }
	virtual Quentin::StateChangeList* getStateChanges(::CORBA::Long changeNum) { return nullptr; }
	virtual Quentin::CopyProgress getCopyRemaining(::CORBA::Long clipID) { return { }; }
	virtual Quentin::CopyProgressList* getCopiesRemaining() { return nullptr; }
	virtual Quentin::CopyMapList* getCopyMap(::CORBA::Long clipID, ::Quentin::FragmentType type, ::CORBA::Long track) { return nullptr; }
	virtual ::CORBA::Boolean deleteCopy(::CORBA::Long clipID) { return nullptr; }
	virtual ::CORBA::Boolean ticketCopy(::CORBA::Long clipID, ::CORBA::Long ticket) { return nullptr; }
	virtual ::CORBA::Boolean tryToTicketCopy(::CORBA::Long clipID) { return nullptr; }
	virtual void unticketCopy(::CORBA::Long clipID) { }
	virtual void addTag(const ::Quentin::RushTag& tagToAdd) { }
	virtual void removeTag(const ::Quentin::RushTag& tagToRemove) { }
	virtual Quentin::RushTagList* getTags(const ::Quentin::RushIdent& rushID, const ::Quentin::WStrings& tagsWanted, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::RushTimecodeList* getTimecodes(const ::Quentin::RushIdent& rushID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual void refresh(::CORBA::Long mode, ::CORBA::Long param) { }
	virtual Quentin::ColumnDescList* getColumnDescriptions() { return nullptr; }
	virtual ::CORBA::Long createClip(const ::Quentin::ClipPropertyList& props, const ::Quentin::ServerFragments& frags) { return 0; }
	virtual ::CORBA::Long createPlacedClip(const ::Quentin::ClipPropertyList& props, const ::Quentin::ServerFragments& frags, ::CORBA::Long poolIdent, ::CORBA::Long ticket, ::CORBA::Long priority) { return 0; }
	virtual Quentin::Longs* findFragsOnPools(const ::Quentin::ServerFragments& frags, const ::Quentin::Longs& pools) { return nullptr; }
	virtual Quentin::Longs* findClipOnPools(::CORBA::Long clipID, const ::Quentin::Longs& pools) { return nullptr; }
	virtual Quentin::ServerFragments* getAllFragments(::CORBA::Long clipID) { return nullptr; }
	virtual Quentin::ServerFragments* getFragments(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getFragmentsWithMode(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish, ::CORBA::Long playMode) { return nullptr; }
	virtual Quentin::ServerFragments* getTypeFragments(::CORBA::Long clipID, ::CORBA::Long trackType) { return nullptr; }
	virtual Quentin::ServerFragments* getSubTypeFragments(::CORBA::Long clipID, ::CORBA::Long trackType, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getTrackFragments(::CORBA::Long clipID, ::CORBA::Long trackType, ::CORBA::Long trackNum) { return nullptr; }
	virtual Quentin::ServerFragments* getSubTrackFragments(::CORBA::Long clipID, ::CORBA::Long trackType, ::CORBA::Long trackNum, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getSourceTimecode(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getRefTimecode(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::WStrings* getClipData(::CORBA::Long clipID, const ::Quentin::WStrings& colsWanted) { return nullptr; }
	virtual void updateClip(::CORBA::Long clipID, const ::Quentin::ClipPropertyList& newColumns) { }
	virtual void setClipProtection(::CORBA::Long clipID, const ::CORBA::WChar* userID, ::Quentin::ProtectMode mode) { }
	virtual ::CORBA::Boolean deleteClip(::CORBA::Long clipID) { return nullptr; }
	virtual ::CORBA::Long trimUnrecorded(::CORBA::Long clipID) { return 0; }
	virtual ::CORBA::Long numberClip(::CORBA::Long clipID, ::CORBA::Long number, ::Quentin::ConflictMode confMode) { return 0; }
	virtual ::CORBA::Long scanNumbers(::CORBA::Long poolID, ::CORBA::Long number, ::Quentin::FindMode mode) { return 0; }
	virtual ::CORBA::Long cloneClip(::CORBA::Long clipID, ::CORBA::Long poolIdent, ::CORBA::Long ticket, ::CORBA::Long priority) { return 0; }
	virtual ::CORBA::Long cloneClipInterZone(::CORBA::Long zoneID, ::CORBA::Long clipID, ::CORBA::Long poolID, ::CORBA::Long priority) { return 0; }
	virtual ::CORBA::Long cloneClipInterZoneWithoutHistory(::CORBA::Long zoneID, ::CORBA::Long clipID, ::CORBA::Long poolID, ::CORBA::Long priority) { return 0; }
	virtual ::CORBA::Long cloneIfNeeded(::CORBA::Long clipID, ::CORBA::Long poolIdent, ::CORBA::Long ticket, ::CORBA::Long priority, ::CORBA::Long expirySecs, ::CORBA::Boolean& copyCreated) { return 0; }
	virtual void replaceContent(::CORBA::Long clipID, ::CORBA::Long contentType, const ::Quentin::ServerFragments& frags) { }
	virtual Quentin::FullClipIDList* createPlaceholder(const ::Quentin::ClipPropertyList& props, const ::Quentin::Longs& pools) { return nullptr; }
	virtual Quentin::FullClipIDList* clonePlaceholder(::CORBA::Long clipID, const ::Quentin::Longs& pools) { return nullptr; }
	virtual Quentin::FullClipIDList* createExtPlaceholder(const ::Quentin::ClipPropertyList& props, const ::Quentin::Longs& pools, const ::Quentin::WStrings& extData) { return nullptr; }
	virtual Quentin::Longs* getPlaceholderPools(::CORBA::Long clipID) { return nullptr; }
	virtual Quentin::PlaceholderDataList* getPlaceholderData(::CORBA::Long clipID) { return nullptr; }
	virtual Quentin::FullClipIDList* fillPlaceholder(::CORBA::Long clipID, const ::Quentin::ServerFragments& frags, const ::Quentin::ClipPropertyList& props, ::CORBA::Long priority) { return nullptr; }
	virtual void fillSinglePlaceholder(const ::Quentin::PlaceholderData& data, const ::Quentin::ServerFragments& frags, const ::Quentin::ClipPropertyList& props, ::CORBA::Long priority) { }
	virtual ::CORBA::Long rushHighWater(const ::Quentin::RushIdent& rushID) { return 0; }
	virtual ::CORBA::Long createDeltaFromClips(::CORBA::Long originalClipID, ::CORBA::Long laterClipID, const ::Quentin::ClipPropertyList& props) { return 0; }
	virtual ::CORBA::Long createDeltaFromFragments(const ::Quentin::ServerFragments& originalFragments, const ::Quentin::ServerFragments& laterFragments, const ::Quentin::ClipPropertyList& props) { return 0; }
	virtual ::CORBA::Long getPoolNumberedClip(::CORBA::Long number, ::CORBA::Long pool) { return 0; }
	virtual Quentin::WStrings* searchClips(const ::Quentin::ClipPropertyList& props, const ::Quentin::WStrings& columns, ::CORBA::Long max) { return nullptr; }
	virtual ::CORBA::Long countClips(const ::Quentin::ClipPropertyList& props) { return 0; }
	virtual Quentin::WStrings* searchClipsWithOffset(const ::Quentin::ClipPropertyList& props, const ::Quentin::WStrings& columns, ::CORBA::Long offset, ::CORBA::Long max) { return nullptr; }
	virtual Quentin::WStrings* orderedSearchClips(const ::Quentin::ClipPropertyList& props, const ::Quentin::WStrings& columns, const ::Quentin::SortOrderList& order, ::CORBA::Long offset, ::CORBA::Long max) { return nullptr; }
	virtual Quentin::Longs* getTaggedClips(const ::CORBA::WChar* tag, const ::CORBA::WChar* keys) { return nullptr; }
	virtual Quentin::FormatInfo* getFormatInfo(::Quentin::FormatCode format) { return nullptr; }
	virtual void getThumbnailSize(::CORBA::Long mode, ::CORBA::Long& width, ::CORBA::Long& height) { }
	virtual ::CORBA::Long requestThumbnails(::CORBA::Long mode, const ::Quentin::PositionData& fragment, ::CORBA::Long offset, ::CORBA::Long stride, ::CORBA::Long count, ::CORBA::Long ident, ::Quentin::ThumbnailListener_ptr listener) { return 0; }
	virtual void abortThumbnails(::CORBA::Long abortID) { }
	virtual ::CORBA::Long getTicket() { return 0; }
	virtual void freeTicket(::CORBA::Long ticket) { }
	virtual ::CORBA::Long getFreeTickets() { return 0; }
	virtual Quentin::WStrings* directQuery(const ::CORBA::WChar* command) { return nullptr; }
	virtual void unregisterAll(::CORBA::Long database, ::CORBA::Long poolID) { }
	virtual ::CORBA::WChar* getServerTime() { return nullptr; }
	virtual ::CORBA::WChar* getSequence(const ::CORBA::WChar* prefix) { return nullptr; }
	virtual Quentin::WStrings* getAreaNames(::CORBA::Boolean search, const ::CORBA::WChar* propWanted) { return nullptr; }
	virtual Quentin::ClipPropertyList* getAreaPropertyList(const ::CORBA::WChar* areaName, ::CORBA::Boolean search) { return nullptr; }
	virtual void setAreaProperties(const char* areaName, ::CORBA::Boolean search, const ::Quentin::ClipPropertyList& properties) { }
	virtual ::CORBA::Long queryFreed(::CORBA::Long poolID, const ::Quentin::Longs& clips) { return 0; }
	virtual ::CORBA::LongLong getFreeProtons(::CORBA::Long poolIdent) { return 0; }
	virtual ::CORBA::Long getFreeFrames(::CORBA::Long poolIdent, const ::Quentin::FormatCodes& formats) { return 0; }
	virtual ::CORBA::Long getZoneNumber();
	virtual Quentin::Longs* getZones(::CORBA::Boolean upOnly);
	virtual Quentin::ZonePortal_ptr getZonePortal(::CORBA::Long zoneID) { return nullptr; }
	virtual ::CORBA::WChar* getZoneName(::CORBA::Long zoneID);
	virtual ::CORBA::Boolean zoneIsRemote(::CORBA::Long zoneID);
	virtual ::CORBA::Long maxAAFRecord() { return 0; }
	virtual void putAAF(::CORBA::Long clipID, ::CORBA::Long record, const ::Quentin::RawData& data) { }
	virtual ::CORBA::Long aafRecordLength(::CORBA::Long clipId) { return 0; }
	virtual void getAAF(::CORBA::Long clipID, ::CORBA::Long record, ::Quentin::RawData_out data) { }
	virtual Quentin::WStrings* getLoggingRoles() { return nullptr; }
	virtual Quentin::WStrings* getLoggingRoleNames(const ::CORBA::WChar* role) { return nullptr; }
	virtual ::CORBA::Long lastAAFRecord(::CORBA::Long clipID) { return 0; }

	virtual ::CORBA::WChar* getProperty(const ::CORBA::WChar* propertyName) { return nullptr; };
	virtual Quentin::WStrings* getPropertyList() { return nullptr; }
};

CORBA::Long ZonePortal_i::getZoneNumber()
{
  return 1000;
}

Quentin::Longs* ZonePortal_i::getZones(::CORBA::Boolean upOnly) {
	Quentin::Longs* zones = new Quentin::Longs;
	zones->length(1);
	(*zones)[0] = 2000;
	return zones;
}

CORBA::Boolean ZonePortal_i::zoneIsRemote(::CORBA::Long zoneID) {
	return (zoneID < 2000) ? CORBA::Boolean(false) : CORBA::Boolean(true);
}

CORBA::WChar* ZonePortal_i::getZoneName(::CORBA::Long zoneID) {
	CORBA::WChar* name = (CORBA::WChar*) malloc(25 * sizeof(CORBA::WChar));
	swprintf(name, 25, L"Zone %i", zoneID);
	return name;
}

Quentin::Longs* ZonePortal_i::getServers(::CORBA::Boolean negateIfDown) {
	Quentin::Longs* servers = new Quentin::Longs;
	servers->length(3);
	(*servers)[0] = 1100;
	(*servers)[1] = 1200;
	(*servers)[2] = 1300;
	return servers;
}

Quentin::Server_ptr ZonePortal_i::getServer(::CORBA::Long serverID) {
	Server_i* myServer = new Server_i;
	Quentin::Server_var server = myServer->_this();
	return server._retn();
}

napi_value runServer(napi_env env, napi_callback_info info) {
	napi_status status;

	int argc = 0;
	CORBA::ORB_var orb = CORBA::ORB_init(argc, nullptr);
  CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

	ZonePortal_i* myecho = new ZonePortal_i();

	PortableServer::ObjectId_var myechoid = poa->activate_object(myecho);
	obj = myecho->_this();
	CORBA::String_var sior(orb->object_to_string(obj));
	printf("%s\n", (char*)sior);
	fflush(stdout);

	myecho->_remove_ref();

	PortableServer::POAManager_var pman = poa->the_POAManager();
	pman->activate();

	orb->run();
	orb->destroy();

	return nullptr;
}
