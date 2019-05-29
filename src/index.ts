import * as request from 'request'
// import { writeFileSync } from 'fs'
//
const quantel = require('../build/Release/quantel_gateway')

// import * as SegfaultHandler from 'segfault-handler'
// SegfaultHandler.registerHandler('crash.log')

export namespace Quantel {

	let isaIOR: Promise<string> | null = null

	export interface ZoneInfo {
		type: string,
		zoneNumber: number,
		zoneName: string,
		isRemote: boolean
	}

	export interface ServerInfo {
		type: string,
		ident: number,
		down: boolean,
		name?: string,
		numChannels?: number,
		pools?: number[],
		portNames?: string[],
		chanPorts?: string[]
	}

	export interface PortRef {
		serverID: number | string,
		portName: string,
	}

	export interface PortInfo extends PortRef {
		type?: string,
		channelNo: number,
		portID?: number,
		audioOnly?: boolean,
		assigned?: boolean,
	}

	export interface PortStatus extends PortRef {
		type: string,
		portID: number,
		refTime: string,
		portTime: string,
		speed: number,
		offset: number,
		flags: number,
		endOfData: number,
		framesUnused: number,
		outputTime: string,
		channels: number[],
	}

	export interface ClipRef {
		clipID: number,
	}

	export interface FragmentRef extends ClipRef {
		start?: number,
		finish?: number,
	}

	export interface ClipPropertyList {
		[ name: string ]: string
	}

	export interface ClipDataSummary {
		type: string,
		ClipID: number,
		Completed: Date | null,
		Created: Date, // ISO-formatted date
		Description: string,
		Frames: string, // TODO ISA type is None ... not sure whether to convert to number
		Owner: string,
		Title: string,
	}

	export interface ClipData extends ClipDataSummary {
		Category: string,
		CloneId: number | null,
		CloneZone: number | null,
		Destination: number | null,
		Expiry: Date | null, // ISO-formatted date
	 	HasEditData: number | null,
		Inpoint: number | null,
		JobID: number | null,
		Modified: string | null,
		NumAudTracks: number | null,
		Number: number | null,
		NumVidTracks: number | null,
		Outpoint: number | null,
		PlaceHolder: boolean,
		PlayAspect: string,
		PoolID: number | null,
		PublishedBy: string,
		Register: string,
		Tape: string,
		Template: number | null,
		UnEdited: number | null,
		PlayMode: string,
		MosActive: boolean,
		Division: string,
		AudioFormats: string,
		VideoFormats: string,
		ClipGUID: string,
		Protection: string,
		VDCPID: string,
		PublishCompleted: Date | null, // ISO-formatted date
	}

	export interface ServerFragment {
		type: string,
		trackNum: number,
		start: number,
		end: number,
	}

	export interface VideoFragment extends ServerFragment {
		rushID: string,
		format: number,
		poolID: number,
		poolFrame: number,
		skew: number,
		rushFrame: number,
	}

	export interface AudioFragment extends VideoFragment { }
	export interface AUXFragment extends VideoFragment { }

	export interface CCFragment extends ServerFragment {
		ccID: string,
		ccType: number,
		effectID: number,
	}

	// TODO extend with the different types
	export interface ServerFragments extends ClipRef {
		type: string,
		fragments: ServerFragment[]
	}

	export interface PortLoadInfo extends PortRef {
		fragments: ServerFragment[],
		offset?: number
	}

	export enum Trigger {
		START = quantel.START,
		STOP = quantel.STOP,
		JUMP = quantel.JUMP,
		TRANSITION = quantel.TRANSITION
	}

	export interface TriggerInfo extends PortRef {
		trigger: Trigger,
		offset?: number
	}

	export interface JumpInfo extends PortRef {
		offset: number
	}

	export interface ThumbnailSize {
		width: number,
		height: number
	}

	export interface ThumbnailOrder extends ClipRef {
		offset: number,
		stride: number,
		count: number,
	}

	export class ConnectError extends Error {
		public statusCode: number
		constructor (message?: string | undefined, statusCode?: number) {
			super(message)
			this.statusCode = statusCode ? statusCode : 500
		}
		get status () {
			return this.statusCode
		}
	}

	export async function getISAReference (ref?: string): Promise<string> {
		if (isaIOR === null || ref) isaIOR = Promise.reject()
		isaIOR = isaIOR.then(x => x, () => new Promise((resolve, reject) => {
			if (ref && ref.endsWith('/')) ref = ref.slice(0, -1)
			if (ref && ref.indexOf(':') < 0) ref = ref + ':2096'
			request(ref ? ref + '/ZoneManager.ior' : 'http://localhost:2096/ZoneManager.ior', (err, res, body) => {
				if (err) {
					reject(err)
				} else {
					if (res.statusCode === 200) {
						resolve(body)
					} else {
						reject(new ConnectError(
							`HTTP request for ISA IOR failed with status ${res.statusCode}: ${res.statusMessage}`,
							res.statusCode))
					}
				}
			})
		}))
		return isaIOR
	}

	export async function testConnection (): Promise<boolean> {
		await getISAReference()
		return quantel.testConnection(await isaIOR)
	}

	export async function listZones (): Promise<ZoneInfo[]> {
		await getISAReference()
		return quantel.listZones(await isaIOR)
	}

	export async function getDefaultZoneInfo (): Promise<ZoneInfo> {
		await getISAReference()
		return quantel.getDefaultZoneInfo(await isaIOR)
	}

	export async function getServers (): Promise<ServerInfo[]> {
		await getISAReference()
		try {
			return quantel.getServers(await isaIOR)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return getServers()
			}
			throw err
		}
	}

	async function checkServer (options: PortRef): Promise<ServerInfo> {
		let servers = quantel.getServers(await isaIOR)
		let server = servers.find((x: ServerInfo) =>
			x.ident === +options.serverID || x.name === options.serverID)
		if (!server) {
			throw new ConnectError(`Not found. Could not find a server with identifier '${options.serverID}'.`, 404)
		}
		options.serverID = server.ident
		return server
	}

	// TODO: allow assignment of more than one channel
	// FIXME: warn on attempt to steal a channel
	export async function createPlayPort (options: PortInfo): Promise<PortInfo> {
		await getISAReference()
		try {
			let server = await checkServer(options)
			if (server.portNames && server.portNames.indexOf(options.portName) >= 0) {
				let portStatus: PortStatus = quantel.getPlayPortStatus(await isaIOR, options)
				if (portStatus.channels.indexOf(options.channelNo) < 0) {
					throw new ConnectError(`Conflict. Port '${options.portName}' on server '${options.serverID}' is already assigned to channels '[${portStatus.channels}]' and cannot be assigned to channel '${options.channelNo}'.`, 409)
				}
				return {
					type: 'PortInfo',
					serverID: options.serverID,
					portName: options.portName,
					channelNo: options.channelNo,
					portID: portStatus.portID,
					assigned: true
				} as PortInfo
			}
			if (server.chanPorts && server.chanPorts[options.channelNo].length !== 0) {
				throw new ConnectError(`Bad request. Cannot assign channel '${options.channelNo}' to port '${options.portName}' on server '${options.serverID}' as it is already assigned to port '${server.chanPorts[options.channelNo]}'.`, 400)
			}
			return quantel.createPlayPort(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return createPlayPort(options)
			}
			throw err
		}
	}

	async function checkServerPort (options: PortRef): Promise<ServerInfo> {
		let server = await checkServer(options)
		if (server.portNames && server.portNames.indexOf(options.portName) < 0) {
			throw new ConnectError(`Not found. Could not find a port called '${options.portName}' on server '${server.name}'.`, 404)
		}
		return server
	}

	export async function getPlayPortStatus (options: PortRef): Promise<PortStatus> {
		await getISAReference()
		try {
			await checkServerPort(options)
			return quantel.getPlayPortStatus(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return getPlayPortStatus(options)
			}
			throw err
		}
	}

	export async function releasePort (options: PortRef): Promise<boolean> {
		await getISAReference()
		try {
			await checkServerPort(options)
			return quantel.releasePort(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return releasePort(options)
			}
			throw err
		}
	}

	export async function getClipData (options: ClipRef): Promise<ClipData> {
		await getISAReference()
		try {
			return quantel.getClipData(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return getClipData(options)
			}
			throw err
		}
	}

	export async function searchClips (options: ClipPropertyList): Promise<ClipDataSummary[]> {
		await getISAReference()
		try {
			return quantel.searchClips(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return searchClips(options)
			}
			throw err
		}
	}

	export async function getFragments (options: FragmentRef): Promise<ServerFragments> {
		await getISAReference()
		try {
			return quantel.getFragments(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return getFragments(options)
			}
			throw err
		}
	}

	export async function loadPlayPort (options: PortLoadInfo): Promise<any> {
		await getISAReference()
		try {
			await checkServerPort(options)
			return quantel.loadPlayPort(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return loadPlayPort(options)
			}
			throw err
		}
	}

	export async function trigger (options: TriggerInfo): Promise<boolean> {
		await getISAReference()
		try {
			await checkServerPort(options)
			return quantel.trigger(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return trigger(options)
			}
			throw err
		}
	}

	export async function jump (options: JumpInfo): Promise<boolean> {
		await getISAReference()
		try {
			await checkServerPort(options)
			return quantel.jump(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return jump(options)
			}
			throw err
		}
	}

	export async function setJump (options: JumpInfo): Promise<boolean> {
		await getISAReference()
		try {
			await checkServerPort(options)
			return quantel.setJump(await isaIOR, options)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return setJump(options)
			}
			throw err
		}
	}

	export async function getThumbnailSize (): Promise<ThumbnailSize> {
		await getISAReference()
		try {
			return quantel.getThumbnailSize(await isaIOR)
		} catch (err) {
			if (err.message.indexOf('OBJECT_NOT_EXIST') >= 0) {
				isaIOR = null
				return getThumbnailSize()
			}
			throw err
		}
	}

	export async function requestThumbnails (options: ThumbnailOrder): Promise<Buffer> {
		await getISAReference()
		let b = quantel.requestThumbnails(await isaIOR, options)
		// writeFileSync(`test${options.offset}.argb`, b)
		return b
	}
}
