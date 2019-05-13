import * as request from 'request'

const quantel = require('../build/Release/quantel_gateway')

export namespace Quantel {

	let isaIOR: Promise<string>

	export interface ZoneInfo {
		zoneNumber: number
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
		return quantel.getZoneInfo(await isaIOR)
	}
}
