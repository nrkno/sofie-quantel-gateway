import * as request from 'request'
import * as SegfaultHandler from 'segfault-handler'

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
}
