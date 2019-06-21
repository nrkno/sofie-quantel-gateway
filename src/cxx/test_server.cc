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

#include "test_server.h"

class Port_i: public POA_Quentin::Port,
				public PortableServer::RefCountServantBase
{
public:
	inline Port_i() { }
	inline Port_i(const CORBA::WChar* ident, CORBA::Long number, CORBA::Long serverID):
	 	ident(ident), number(number), serverID(serverID) { }
	virtual ~Port_i() { }
	virtual void changeFlags(::CORBA::Long mask, ::CORBA::Long newFlags) { return; }
	virtual ::CORBA::Boolean setMode(::Quentin::Port::PortMode newMode);
	virtual void reset() { return; }
	virtual void resetTriggers() { return; }
	virtual void resetTracks() { return; }
	virtual void setTrackLimits(::CORBA::Long startFrame, ::CORBA::Long endFrame) { return; }
	virtual Quentin::Port::GeneralPortStatus getStatus();
	virtual void reportStatus(::Quentin::PortListener_ptr listener, ::CORBA::Long interval, ::CORBA::Long noteMask) { return; }
	virtual ::CORBA::Boolean setTrigger(::CORBA::Long trigger, ::Quentin::Port::TriggerMode mode, ::CORBA::Long param);
	virtual ::CORBA::Boolean actionAtTrigger(::CORBA::Long trigger, ::Quentin::Port::TriggerAction action);
	virtual Quentin::Port::TriggerState getTrigger(::CORBA::Long trigger) { return {}; }
	virtual Quentin::Port::TriggerStates* getTriggers() { return nullptr; }
	virtual Quentin::ServerFragments* getPortFragments(::CORBA::Long start, ::CORBA::Long finish);
	virtual Quentin::ServerFragments* getPortTypeFragments(::CORBA::Long start, ::CORBA::Long finish, ::CORBA::Long fragType) { return nullptr; }
	virtual Quentin::ServerFragments* getPortTrackFragments(::CORBA::Long start, ::CORBA::Long finish, ::CORBA::Long fragType, ::CORBA::Long trackNum) { return nullptr; }
	virtual void getThumbnailSize(::CORBA::Long mode, ::CORBA::Long& width, ::CORBA::Long& height) { return; }
	virtual void setThumbnailListener(::CORBA::Long mode, ::CORBA::Long chanNum, ::CORBA::Long ident, ::Quentin::ThumbnailListener_ptr listener, ::CORBA::Long minInterval) { return; }
	virtual ::CORBA::Long requestThumbnails(::CORBA::Long mode, ::CORBA::Long chanNum, ::CORBA::Long offset, ::CORBA::Long stride, ::CORBA::Long count, ::CORBA::Long ident, ::Quentin::ThumbnailListener_ptr listener) { return 0; }
	virtual void abortThumbnails(::CORBA::Long abortID) { return; }
	virtual Quentin::WStrings* controllerNames() { return nullptr; }
	virtual Quentin::EffectController_ptr getController(::CORBA::Long trackNum) { return nullptr; }
	virtual Quentin::PortInfo* getPortInfo() { return nullptr; }
	virtual Quentin::DirectoryViewer_ptr getDirViewer(::CORBA::Long timeoutSecs, const ::CORBA::WChar* viewerName) { return nullptr; }
	virtual Quentin::DirectoryViewer_ptr getPoolDirViewer(::CORBA::Long poolID, ::CORBA::Long timeoutSecs, const ::CORBA::WChar* viewerName) { return nullptr; }
	virtual Quentin::ConfigDescriptionList* getConfigurations(::CORBA::Long channel, ::Quentin::FragmentType type, ::CORBA::Boolean forPlay) { return nullptr; }
	virtual Quentin::Longs* getDefaultConfigurations(::CORBA::Long channel) { return nullptr; }
	virtual Quentin::Longs* getCurrentConfigurations(::CORBA::Long channel) { return nullptr; }
	virtual void configure(::CORBA::Long channel, const ::Quentin::Longs& configurations) { return; }
	virtual ::CORBA::Boolean assignChannel(::CORBA::Long chanNum, ::CORBA::Long flags);
	virtual void assignTransitionPort(::Quentin::Port_ptr transitionPort) { return; }
	virtual Quentin::Port_ptr getTransitionPort() { return nullptr; }
	virtual Quentin::Longs* getChannels();
	virtual void release() { return; }
	virtual void load(::CORBA::Long offset, const ::Quentin::ServerFragments& fragments) { return; }
	virtual void insertBlank(::CORBA::Long start, ::CORBA::Long frames) { return; }
	virtual ::CORBA::Boolean remove(::CORBA::Long start, ::CORBA::Long frames) { return false; }
	virtual ::CORBA::Boolean wipe(::CORBA::Long start, ::CORBA::Long frames) { return ::CORBA::Boolean(true); }
	virtual void jump(::CORBA::Long offset, ::CORBA::Boolean disablePreload) { return; }
	virtual void jumpRelative(::CORBA::Long offset) { return; }
	virtual void setJump(::CORBA::Long offset) { return; }
	virtual void setTransition(::Quentin::Port::TransitionType type, ::CORBA::Long frames, ::CORBA::Boolean autoPlay) { return; }
	virtual void setSpeed(::CORBA::Float newSpeed) { return; }
	virtual ::CORBA::Long jogAudio(::CORBA::Long frames) { return 0; }
	virtual void jogSubFrames(::CORBA::Long subFrameTicks) { return; }
	virtual void setInputAudioPatch(const ::Quentin::AudioPatchInfoList& patches, ::CORBA::Boolean preview) { return; }
	virtual void setOutputAudioPatch(::CORBA::Long startFrame, ::CORBA::Long endFrame, const ::Quentin::AudioPatchInfoList& patches) { return; }
	virtual ::CORBA::Long extendSpace(::CORBA::Long poolID, ::CORBA::Long totalFrames) { return 0; }
	virtual ::CORBA::Long forget(::CORBA::Long offset) { return 0; }
	virtual void setFlags(::CORBA::Long track, ::CORBA::Long newFlags) { return; }
	virtual void setCrop(::CORBA::Long track, ::CORBA::Long cropLeft, ::CORBA::Long cropTop, ::CORBA::Long cropWidth, ::CORBA::Long cropHeight) { return; }
	virtual void setAspect(::CORBA::Long track, ::CORBA::Long width, ::CORBA::Long height) { return; }
	virtual void setOriginator(const ::CORBA::WChar* originator) { return; }
	virtual void setRecordTimecodes(const ::Quentin::RushTimecodeList& timecodes) { return; }
	virtual Quentin::RushIdent getRushRecording(::Quentin::FragmentType type, ::CORBA::Long track) { return {}; }
	virtual void setOverlayClipTitle(const ::CORBA::WChar* clipname, ::Quentin::Port::OverlayTextColour colour) { return; }
	virtual void setOverlayIndicator(const ::CORBA::WChar* indicator, ::CORBA::Boolean active) { return; }
	virtual void setOverlayTallyID(::CORBA::Long id, ::CORBA::Boolean numeric) { return; }

	virtual ::CORBA::WChar* getProperty(const ::CORBA::WChar* propertyName) { return nullptr; }
	virtual Quentin::WStrings* getPropertyList() { return nullptr; }

private:
	const ::CORBA::WChar* ident;
	::CORBA::Long number;
	::CORBA::Long serverID;
};

CORBA::Boolean Port_i::assignChannel(CORBA::Long chanNum, CORBA::Long flags) {
	return CORBA::Boolean(chanNum >= 2);
}

CORBA::Boolean Port_i::setMode(::Quentin::Port::PortMode newMode) {
	return (newMode != Quentin::Port::PortMode::recording);
}

CORBA::Boolean Port_i::actionAtTrigger(CORBA::Long trigger, Quentin::Port::TriggerAction action) {
	switch (trigger) {
		case START:
			return (action == Quentin::Port::trActStart);
		case STOP:
			return (action == Quentin::Port::trActStop);
		case JUMP:
			return (action == Quentin::Port::trActJump);
		default:
			return false;
	}
}

Quentin::Port::GeneralPortStatus Port_i::getStatus() {
	Quentin::PortListener::PlayPortStatus_var pps;
	pps->portNumber = 42;
	pps->flags = 3;
	pps->refTime = 0x10111213;
	pps->portTime = 0x13121110;
	pps->framesUnused = 43;
	pps->offset = 44;
	pps->endOfData = 45;
	pps->speed = 0.5;
	pps->outputTime = 0x23595924;
	Quentin::Port::GeneralPortStatus_var gps = {};
	gps->playStatus(pps);
	return gps._retn();
}

Quentin::Longs* Port_i::getChannels() {
	Quentin::Longs* channels = new Quentin::Longs;
	channels->length(1);
	(*channels)[0] = 1;
	return channels;
}

Quentin::ServerFragments* Port_i::getPortFragments(CORBA::Long start, CORBA::Long finish) {
	Quentin::ServerFragments* frags = new Quentin::ServerFragments;
	frags->length(2);

	// VideoFragment
	Quentin::ServerFragment sfv = {};
	Quentin::PositionData vfd = {};
	Quentin::ServerFragmentData sfvd;

	sfv.trackNum = 0;
	sfv.start = start;
	sfv.finish = finish > 1000 ? 1000 : finish;
	vfd.format = 90;
	vfd.poolID = 11;
	vfd.poolFrame = 123;
	vfd.skew = 42;
	vfd.rushFrame = 543210;
	vfd.rushID = {
		(CORBA::LongLong) 0x0123456789abcdef,
		(CORBA::LongLong) 0xfedcba9876543210 };
	sfvd.videoFragmentData(vfd);
	sfv.fragmentData = sfvd;
	(*frags)[0] = sfv;

	// AudioFragment
	Quentin::ServerFragment sfa = {};
	Quentin::PositionData afd = {};
	Quentin::ServerFragmentData sfad;

	sfa.trackNum = 0;
	sfa.start = start;
	sfa.finish = finish > 1000 ? 1000 : finish;
	afd.format = 73;
	afd.poolID = 11;
	afd.poolFrame = 321;
	afd.skew = 24;
	afd.rushFrame = 123456;
	afd.rushID = {
		(CORBA::LongLong) 0xfedcba9876543210,
		(CORBA::LongLong) 0x0123456789abcdef };
	sfad.audioFragmentData(afd);
	sfa.fragmentData = sfad;
	(*frags)[1] = sfa;

	return frags;
}

CORBA::Boolean Port_i::setTrigger(CORBA::Long trigger, Quentin::Port::TriggerMode mode, CORBA::Long param) {
	switch (trigger) {
		case START:
		case STOP:
		case JUMP:
			return true;
		default:
			return false;
	}
}

class Server_i: public POA_Quentin::Server,
        public PortableServer::RefCountServantBase
{
public:
  inline Server_i() {}
	inline Server_i(int32_t id): serverID(id) { };
  virtual ~Server_i() {}
	virtual Quentin::ServerInfo* getServerInfo();
	virtual Quentin::Port_ptr getPort(const ::CORBA::WChar* ident, ::CORBA::Long number);
	virtual Quentin::WStrings* getPortNames();
	virtual Quentin::WStrings* getChanPorts();
	virtual ::CORBA::LongLong getFreeProtons(::CORBA::Long poolIdent) { return 0; }
	virtual Quentin::Longs* getPools() { return nullptr; }
	virtual Quentin::Timecode getRefTime() { return Quentin::Timecode(); }
	virtual Quentin::ConfigDescriptionList* getConfigurations(::CORBA::Long channel, ::Quentin::FragmentType type, ::CORBA::Boolean forPlay) { return nullptr; }
	virtual Quentin::Longs* getDefaultConfigurations(::CORBA::Long channel) { return nullptr; }
	virtual Quentin::Longs* getCurrentConfigurations(::CORBA::Long channel) { return nullptr; }
	virtual Quentin::ServerCapabilities* getServerCapabilities() { return nullptr; }

	virtual ::CORBA::WChar* getProperty(const ::CORBA::WChar* propertyName) { return nullptr; }
	virtual Quentin::WStrings* getPropertyList() { return nullptr; }
private:
	int32_t serverID = 0;
};

Quentin::ServerInfo* Server_i::getServerInfo() {
	Quentin::ServerInfo* serverInfo = new Quentin::ServerInfo;
	serverInfo->ident = serverID;
	CORBA::WChar* name = (CORBA::WChar*) malloc(25 * sizeof(CORBA::WChar));
	swprintf(name, 25, L"Server %i", serverID);
	serverInfo->name = name;
	serverInfo->down = (serverID == 1200);
	switch (serverID) {
		case 1100: serverInfo->numChannels = (CORBA::Long) 4; break;
		case 1200: serverInfo->numChannels = (CORBA::Long) 2; break;
		case 1300: serverInfo->numChannels = (CORBA::Long) 3; break;
		default: serverInfo->numChannels = (CORBA::Long) 1; break;
	}
	serverInfo->pools.length(1);
	serverInfo->pools[0] = serverID / 100;
	return serverInfo;
}

Quentin::WStrings* Server_i::getPortNames() {
	Quentin::WStrings* portNames = new Quentin::WStrings;
	CORBA::Long numPorts = 0;
	switch (serverID) {
		case 1100: numPorts = (CORBA::Long) 2; break;
		case 1200: numPorts = (CORBA::Long) 1; break;
		case 1300: numPorts = (CORBA::Long) 2; break;
		default: numPorts = (CORBA::Long) 1; break;
	}

	portNames->length(numPorts);
	for ( int32_t x = 0 ; x < numPorts ; x++ ) {
		(*portNames)[x] = (CORBA::WChar*) malloc(10 * sizeof(CORBA::WChar));
		swprintf((*portNames)[x], 10, L"Port %i", x+1);
	}
	return portNames;
}

Quentin::WStrings* Server_i::getChanPorts() {
	Quentin::WStrings* portNames = new Quentin::WStrings;
	CORBA::Long numPorts = 0;
	CORBA::Long numChannels = 0;
	switch (serverID) {
		case 1100: numChannels = (CORBA::Long) 4; numPorts = (CORBA::Long) 2; break;
		case 1200: numChannels = (CORBA::Long) 2; numPorts = (CORBA::Long) 1; break;
		case 1300: numChannels = (CORBA::Long) 3; numPorts = (CORBA::Long) 2; break;
		default: numChannels = (CORBA::Long) 1; numPorts = (CORBA::Long) 1; break;
	}

	portNames->length(numChannels);
	for ( int32_t x = 0 ; x < numChannels ; x++ ) {
		(*portNames)[x] = (CORBA::WChar*) malloc(10 * sizeof(CORBA::WChar));
		if (x < numPorts) {
			swprintf((*portNames)[x], 10, L"Port %i", x+1);
		} else {
			swprintf((*portNames)[x], 10, L"");
		}
	}
	return portNames;
	return nullptr;
}

Quentin::Port_ptr Server_i::getPort(const CORBA::WChar* ident, CORBA::Long number) {
	Port_i* myPort = new Port_i(ident, number, serverID);
	Quentin::Port_var port = myPort->_this();
	return port._retn();
}

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
	virtual ::CORBA::Boolean addStateChangeListener(::Quentin::StateChangeListener_ptr listener, ::CORBA::Long flags) { return false; }
	virtual ::CORBA::Boolean addNamedStateChangeListener(const ::CORBA::WChar* listenerName, ::Quentin::StateChangeListener_ptr listener, ::CORBA::Long flags, ::CORBA::Long interval) { return false; }
	virtual ::CORBA::Boolean removeStateChangeListener(::Quentin::StateChangeListener_ptr listener) { return false; }
	virtual Quentin::StateChangeList* getStateChanges(::CORBA::Long changeNum) { return nullptr; }
	virtual Quentin::CopyProgress getCopyRemaining(::CORBA::Long clipID) { return { }; }
	virtual Quentin::CopyProgressList* getCopiesRemaining() { return nullptr; }
	virtual Quentin::CopyMapList* getCopyMap(::CORBA::Long clipID, ::Quentin::FragmentType type, ::CORBA::Long track) { return nullptr; }
	virtual ::CORBA::Boolean deleteCopy(::CORBA::Long clipID) { return false; }
	virtual ::CORBA::Boolean ticketCopy(::CORBA::Long clipID, ::CORBA::Long ticket) { return false; }
	virtual ::CORBA::Boolean tryToTicketCopy(::CORBA::Long clipID) { return false; }
	virtual void unticketCopy(::CORBA::Long clipID) { }
	virtual void addTag(const ::Quentin::RushTag& tagToAdd) { }
	virtual void removeTag(const ::Quentin::RushTag& tagToRemove) { }
	virtual Quentin::RushTagList* getTags(const ::Quentin::RushIdent& rushID, const ::Quentin::WStrings& tagsWanted, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::RushTimecodeList* getTimecodes(const ::Quentin::RushIdent& rushID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual void refresh(::CORBA::Long mode, ::CORBA::Long param) { }
	virtual Quentin::ColumnDescList* getColumnDescriptions();
	virtual ::CORBA::Long createClip(const ::Quentin::ClipPropertyList& props, const ::Quentin::ServerFragments& frags) { return 0; }
	virtual ::CORBA::Long createPlacedClip(const ::Quentin::ClipPropertyList& props, const ::Quentin::ServerFragments& frags, ::CORBA::Long poolIdent, ::CORBA::Long ticket, ::CORBA::Long priority) { return 0; }
	virtual Quentin::Longs* findFragsOnPools(const ::Quentin::ServerFragments& frags, const ::Quentin::Longs& pools) { return nullptr; }
	virtual Quentin::Longs* findClipOnPools(::CORBA::Long clipID, const ::Quentin::Longs& pools) { return nullptr; }
	virtual Quentin::ServerFragments* getAllFragments(::CORBA::Long clipID);
	virtual Quentin::ServerFragments* getFragments(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish);
	virtual Quentin::ServerFragments* getFragmentsWithMode(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish, ::CORBA::Long playMode) { return nullptr; }
	virtual Quentin::ServerFragments* getTypeFragments(::CORBA::Long clipID, ::CORBA::Long trackType) { return nullptr; }
	virtual Quentin::ServerFragments* getSubTypeFragments(::CORBA::Long clipID, ::CORBA::Long trackType, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getTrackFragments(::CORBA::Long clipID, ::CORBA::Long trackType, ::CORBA::Long trackNum) { return nullptr; }
	virtual Quentin::ServerFragments* getSubTrackFragments(::CORBA::Long clipID, ::CORBA::Long trackType, ::CORBA::Long trackNum, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getSourceTimecode(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::ServerFragments* getRefTimecode(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish) { return nullptr; }
	virtual Quentin::WStrings* getClipData(::CORBA::Long clipID, const ::Quentin::WStrings& colsWanted);
	virtual void updateClip(::CORBA::Long clipID, const ::Quentin::ClipPropertyList& newColumns) { }
	virtual void setClipProtection(::CORBA::Long clipID, const ::CORBA::WChar* userID, ::Quentin::ProtectMode mode) { }
	virtual ::CORBA::Boolean deleteClip(::CORBA::Long clipID) { return false; }
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
	virtual Quentin::WStrings* searchClips(const ::Quentin::ClipPropertyList& props, const ::Quentin::WStrings& columns, ::CORBA::Long max);
	virtual ::CORBA::Long countClips(const ::Quentin::ClipPropertyList& props) { return 0; }
	virtual Quentin::WStrings* searchClipsWithOffset(const ::Quentin::ClipPropertyList& props, const ::Quentin::WStrings& columns, ::CORBA::Long offset, ::CORBA::Long max) { return nullptr; }
	virtual Quentin::WStrings* orderedSearchClips(const ::Quentin::ClipPropertyList& props, const ::Quentin::WStrings& columns, const ::Quentin::SortOrderList& order, ::CORBA::Long offset, ::CORBA::Long max) { return nullptr; }
	virtual Quentin::Longs* getTaggedClips(const ::CORBA::WChar* tag, const ::CORBA::WChar* keys) { return nullptr; }
	virtual Quentin::FormatInfo* getFormatInfo(::Quentin::FormatCode format);
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
	/* Quentin::Longs_var servers;
	servers->length(3);
	servers[0] = 1100;
	servers[1] = 1200;
	servers[2] = 1300;
	return servers._retn(); */
}

Quentin::Server_ptr ZonePortal_i::getServer(::CORBA::Long serverID) {
	Server_i* myServer = new Server_i(serverID);
	Quentin::Server_var server = myServer->_this();
	return server._retn();
}

Quentin::ColumnDescList* ZonePortal_i::getColumnDescriptions() {
	Quentin::ColumnDescList* cols = new Quentin::ColumnDescList;
	cols->length(4);
	// (*cols)[0] = Quentin::ColumnDesc_var();
	(*cols)[0].columnName = CORBA::WString_var(L"ClipID");
	(*cols)[0].columnType = CORBA::WString_var(L"Number");
	(*cols)[0].alias = CORBA::WString_var(L"AltClipID");
	(*cols)[0].alterable = false;
	(*cols)[0].creatable = false;
	(*cols)[0].searchable = true;
	(*cols)[0].clones = false;

	// (*cols)[1] = Quentin::ColumnDesc_var();
	(*cols)[1].columnName = CORBA::WString_var(L"Created");
	(*cols)[1].columnType = CORBA::WString_var(L"Date");
	(*cols)[1].alias = CORBA::WString_var(L"AltCreated");
	(*cols)[1].alterable = false;
	(*cols)[1].creatable = false;
	(*cols)[1].searchable = true;
	(*cols)[1].clones = true;

	// (*cols)[2] = Quentin::ColumnDesc_var();
	(*cols)[2].columnName = CORBA::WString_var(L"PlaceHolder");
	(*cols)[2].columnType = CORBA::WString_var(L"Boolean");
	(*cols)[2].alias = CORBA::WString_var(L"");
	(*cols)[2].alterable = false;
	(*cols)[2].creatable = false;
	(*cols)[2].searchable = true;
	(*cols)[2].clones = true;

	// (*cols)[3] = Quentin::ColumnDesc_var();
	(*cols)[3].columnName = CORBA::WString_var(L"Title");
	(*cols)[3].columnType = CORBA::WString_var(L"String");
	(*cols)[3].alias = CORBA::WString_var(L"AltTitle");
	(*cols)[3].alterable = true;
	(*cols)[3].creatable = true;
	(*cols)[3].searchable = true;
	(*cols)[3].clones = true;

	return cols;
}

Quentin::WStrings* ZonePortal_i::getClipData(CORBA::Long clipID, const Quentin::WStrings& colsWanted) {
	Quentin::WStrings* clipInfo = new Quentin::WStrings;
	if (clipID != 2) {
		throw Quentin::BadIdent(Quentin::BadIdentReason::clipNotKnown, clipID);
	}
	clipInfo->length(4);
	(*clipInfo)[0] = CORBA::WString_var(L"2");
	(*clipInfo)[1] = CORBA::WString_var(L"1560366960000");
	(*clipInfo)[2] = CORBA::WString_var(L"false");
	(*clipInfo)[3] = CORBA::WString_var(L"Once upon a time in Quantel");
	return clipInfo;
}

Quentin::WStrings* ZonePortal_i::searchClips(const Quentin::ClipPropertyList& properties, const Quentin::WStrings& columns, CORBA::Long max) {
	Quentin::WStrings* result = new Quentin::WStrings;
	Quentin::ClipProperty theProp = properties[0];
	if (std::wstring(theProp.name) == L"Fred") {
		throw Quentin::BadColumnData(theProp.name, theProp.value);
	}
	if (std::wstring(theProp.name) == L"PoolID") { theProp = properties[1]; }
	if (std::wstring(theProp.value) != L"Once upon*" &&
			std::wstring(theProp.value) != L"e977435806f24b37aed871bf15a2eef9") {
		result->length(0);
		// printf("Got into here.\n"); fflush(stdout);
		return result;
	}
	result->length(columns.length());
	for ( uint32_t x = 0 ; x < result->length() ; x++ ) {
		if (std::wstring(columns[x]) == L"ClipID") {
			(*result)[x] = CORBA::WString_var(L"2"); continue;
		}
		if (std::wstring(columns[x]) == L"ClipGUID") {
			(*result)[x] = CORBA::WString_var(L"e977435806f24b37aed871bf15a2eef9"); continue;
		}
		if (std::wstring(columns[x]) == L"CloneId") {
			(*result)[x] = CORBA::WString_var(L"2"); continue;
		}
		if (std::wstring(columns[x]) == L"Completed") {
			(*result)[x] = CORBA::WString_var(L"1560366960000"); continue;
		}
		if (std::wstring(columns[x]) == L"Created") {
			(*result)[x] = CORBA::WString_var(L"1560366960000"); continue;
		}
		if (std::wstring(columns[x]) == L"Description") {
			(*result)[x] = CORBA::WString_var(L"This is the best programme ever to be produced."); continue;
		}
		if (std::wstring(columns[x]) == L"Frames") {
			(*result)[x] = CORBA::WString_var(L"1234"); continue;
		}
		if (std::wstring(columns[x]) == L"Owner") {
			(*result)[x] = CORBA::WString_var(L"Mine Hands Off"); continue;
		}
		if (std::wstring(columns[x]) == L"PoolID") {
			(*result)[x] = CORBA::WString_var(L"11"); continue;
		}
		if (std::wstring(columns[x]) == L"Title") {
			(*result)[x] = CORBA::WString_var(L"Once upon a time in Quantel"); continue;
		}
		throw Quentin::BadColumnData(columns[x], columns[x]);
	}
	return result;
}

Quentin::ServerFragments* ZonePortal_i::getAllFragments(CORBA::Long clipID) {
	if (clipID != 2) {
		throw Quentin::BadIdent(Quentin::BadIdentReason::clipNotKnown, clipID);
	}
	Quentin::ServerFragments* frags = new Quentin::ServerFragments;
	frags->length(2);

  // VideoFragment
	Quentin::ServerFragment sfv = {};
	Quentin::PositionData vfd = {};
	Quentin::ServerFragmentData sfvd;

	sfv.trackNum = 0;
	sfv.start = 0;
	sfv.finish = 1000;
	vfd.format = 90;
	vfd.poolID = 11;
	vfd.poolFrame = 123;
	vfd.skew = 42;
	vfd.rushFrame = 543210;
	vfd.rushID = {
		(CORBA::LongLong) 0x0123456789abcdef,
		(CORBA::LongLong) 0xfedcba9876543210 };
	sfvd.videoFragmentData(vfd);
	sfv.fragmentData = sfvd;
	(*frags)[0] = sfv;

  // AudioFragment
	Quentin::ServerFragment sfa = {};
	Quentin::PositionData afd = {};
	Quentin::ServerFragmentData sfad;

	sfa.trackNum = 0;
	sfa.start = 0;
	sfa.finish = 1000;
	afd.format = 73;
	afd.poolID = 11;
	afd.poolFrame = 321;
	afd.skew = 24;
	afd.rushFrame = 123456;
	afd.rushID = {
		(CORBA::LongLong) 0xfedcba9876543210,
		(CORBA::LongLong) 0x0123456789abcdef };
	sfad.audioFragmentData(afd);
	sfa.fragmentData = sfad;
	(*frags)[1] = sfa;

	// TODO other fragment types

	return frags;
}

Quentin::ServerFragments* ZonePortal_i::getFragments(::CORBA::Long clipID, ::CORBA::Long start, ::CORBA::Long finish) {
	if (clipID != 2) {
		throw Quentin::BadIdent(Quentin::BadIdentReason::clipNotKnown, clipID);
	}
	Quentin::ServerFragments* frags = new Quentin::ServerFragments;
	frags->length(2);

  // VideoFragment
	Quentin::ServerFragment sfv = {};
	Quentin::PositionData vfd = {};
	Quentin::ServerFragmentData sfvd;

	sfv.trackNum = 0;
	sfv.start = start;
	sfv.finish = finish;
	vfd.format = 90;
	vfd.poolID = 11;
	vfd.poolFrame = 123;
	vfd.skew = 42;
	vfd.rushFrame = 543345;
	vfd.rushID = {
		(CORBA::LongLong) 0x0123456789abcdef,
		(CORBA::LongLong) 0xfedcba9876543210 };
	sfvd.videoFragmentData(vfd);
	sfv.fragmentData = sfvd;
	(*frags)[0] = sfv;

  // AudioFragment
	Quentin::ServerFragment sfa = {};
	Quentin::PositionData afd = {};
	Quentin::ServerFragmentData sfad;

	sfa.trackNum = 0;
	sfa.start = start;
	sfa.finish = finish;
	afd.format = 73;
	afd.poolID = 11;
	afd.poolFrame = 321;
	afd.skew = 24;
	afd.rushFrame = 123654;
	afd.rushID = {
		(CORBA::LongLong) 0xfedcba9876543210,
		(CORBA::LongLong) 0x0123456789abcdef };
	sfad.audioFragmentData(afd);
	sfa.fragmentData = sfad;
	(*frags)[1] = sfa;

	// TODO other fragment types
	return frags;
}

Quentin::FormatInfo* ZonePortal_i::getFormatInfo(Quentin::FormatCode format) {
	if (format != 90) {
			throw Quentin::BadIdent(Quentin::BadIdentReason::formatNotKnown, format);
	}
	Quentin::FormatInfo* fi = new Quentin::FormatInfo;
	fi->formatNumber = 90;
	fi->essenceType = (Quentin::FragmentType) 0;
	fi->frameRate = 25;
	fi->height = 576;
	fi->width = 720;
	fi->samples = 0;
	fi->formatName = L"Legacy 9E Mpeg 40 576i25";
	fi->layoutName = L"720x576i25";
	fi->compressionName = L"Mpeg-2";
	return fi;
}

napi_value runServer(napi_env env, napi_callback_info info) {
	// napi_status status;

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
