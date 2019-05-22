import * as request from 'request'
import * as SegfaultHandler from 'segfault-handler'
//
const quantel = require('../build/Release/quantel_gateway')
SegfaultHandler.registerHandler('crash.log')

export namespace Quantel {

	let isaIOR: Promise<string>

	export interface ZoneInfo {
		type: string,
		zoneNumber: number,
		zoneName: string
	}

	export interface ServerInfo {
		type: string,
		ident: number,
		down: boolean,
		name?: string,
		numChannels?: number,
		pools?: Array<number>,
		portNames?: Array<string>,
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
		speed: number,
		offset: number,
		flags: number,
		endOfData: number,
		framesUnused: number,
	}

	export interface ClipRef {
		clipID: number,
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
		fragments: Array<ServerFragment>
	}

	export interface PortLoadInfo extends PortRef {
		fragments: Array<ServerFragment>
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

	export async function getISAReference (ref?: string): Promise<string> {
		isaIOR = new Promise((resolve, reject) => {
			// TODO better port resolution
			request(ref ? ref + '/ZoneManager.ior' : 'http://localhost:2096/ZoneManager.ior', (err, res, body) => {
				if (err) {
					reject(err)
				} else {
					if (res.statusCode === 200) {
						resolve(body)
					} else {
						reject(new Error(`HTTP request for ISA IOR failed with status ${res.statusCode}.`))
					}
				}
			})
		})
		return isaIOR
	}

	export async function testConnection (): Promise<boolean> {
		if (!isaIOR) await getISAReference()
		return quantel.testConnection(await isaIOR)
	}

	export async function getZoneInfo (): Promise<ZoneInfo> {
		if (!isaIOR) await getISAReference()
		return quantel.getZoneInfo(await isaIOR)
	}

	export async function getServers (): Promise<Array<ServerInfo>> {
		if (!isaIOR) await getISAReference()
		return quantel.getServers(await isaIOR)
	}

	export async function createPlayPort (options: PortInfo): Promise<PortInfo> {
		if (!isaIOR) await getISAReference()
		return quantel.createPlayPort(await isaIOR, options)
	}

	export async function getPlayPortStatus (options: PortRef): Promise<any> {
		if (!isaIOR) await getISAReference()
		return quantel.getPlayPortStatus(await isaIOR, options)
	}

	export async function releasePort (options: PortRef): Promise<boolean> {
		if (!isaIOR) await getISAReference()
		return quantel.releasePort(await isaIOR, options)
	}

	export async function getAllFragments (options: ClipRef): Promise<ServerFragments> {
		if (!isaIOR) await getISAReference()
		return quantel.getAllFragments(await isaIOR, options)
	}

	export async function loadPlayPort (options: PortLoadInfo): Promise<any> {
		if (!isaIOR) await getISAReference()
		return quantel.loadPlayPort(await isaIOR, options)
	}

	export async function trigger (options: TriggerInfo): Promise<boolean> {
		if (!isaIOR) await getISAReference()
		return quantel.trigger(await isaIOR, options)
	}

	export async function jump (options: JumpInfo): Promise<boolean> {
		if (!isaIOR) await getISAReference()
		return quantel.jump(await isaIOR, options)
	}

	export async function setJump (options: JumpInfo): Promise<boolean> {
		if (!isaIOR) await getISAReference()
		return quantel.setJump(await isaIOR, options)
	}

	export async function getThumbnailSize (): Promise<ThumbnailSize> {
		if (!isaIOR) await getISAReference()
		return quantel.getThumbnailSize(await isaIOR)
	}

	export async function requestThumbnails (): Promise<ThumbnailSize> {
		if (!isaIOR) await getISAReference()
		return quantel.requestThumbnails(await isaIOR)
	}
}
