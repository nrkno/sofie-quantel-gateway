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
	}

	export interface PortRef {
		serverID: number,
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

	export async function createPlayPort (options: PortInfo): Promise<PortInfo> {
		await getISAReference()
		return quantel.createPlayPort(await isaIOR, options)
	}

	export async function getPlayPortStatus (options: PortRef): Promise<any> {
		await getISAReference()
		return quantel.getPlayPortStatus(await isaIOR, options)
	}

	export async function releasePort (options: PortRef): Promise<boolean> {
		await getISAReference()
		return quantel.releasePort(await isaIOR, options)
	}

	export async function getClipData (options: ClipRef): Promise<ClipData> {
		await getISAReference()
		return quantel.getClipData(await isaIOR, options)
	}

	export async function searchClips (options: ClipPropertyList): Promise<ClipDataSummary[]> {
		await getISAReference()
		return quantel.searchClips(await isaIOR, options)
	}

	export async function getFragments (options: FragmentRef): Promise<ServerFragments> {
		await getISAReference()
		return quantel.getFragments(await isaIOR, options)
	}

	export async function loadPlayPort (options: PortLoadInfo): Promise<any> {
		await getISAReference()
		return quantel.loadPlayPort(await isaIOR, options)
	}

	export async function trigger (options: TriggerInfo): Promise<boolean> {
		await getISAReference()
		return quantel.trigger(await isaIOR, options)
	}

	export async function jump (options: JumpInfo): Promise<boolean> {
		await getISAReference()
		return quantel.jump(await isaIOR, options)
	}

	export async function setJump (options: JumpInfo): Promise<boolean> {
		await getISAReference()
		return quantel.setJump(await isaIOR, options)
	}

	export async function getThumbnailSize (): Promise<ThumbnailSize> {
		await getISAReference()
		return quantel.getThumbnailSize(await isaIOR)
	}

	export async function requestThumbnails (options: ThumbnailOrder): Promise<Buffer> {
		await getISAReference()
		let b = quantel.requestThumbnails(await isaIOR, options)
		// writeFileSync(`test${options.offset}.argb`, b)
		return b
	}
}
