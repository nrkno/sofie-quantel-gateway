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

import Koa from 'koa'
import Router from 'koa-router'
import bodyParser from 'koa-bodyparser'
import { Quantel } from '.'
import { StatusResponse, ExternalStatus } from './systemStatus'
import yargs from 'yargs'
import { Server, get } from 'http'
import PQueue from 'p-queue'

const queue = new PQueue({
	concurrency: 1,
	timeout: 10000,
	throwOnTimeout: true
})
import { performance } from 'perf_hooks'

let debug = false
export function setDebug (d: boolean) {
	debug = d
}

setDebug(process.env.DEBUG === '1')

let TIMEOUT_TIME = 3000 // 3 seconds
export function debugLog (...args: any[]) {
	if (debug) console.log(...args)
}
export function infoLog (...args: any[]) {
	console.log(...args)
}
export function errorLog (...args: any[]) {
	console.error(...args)
}

infoLog('Starting Quantel Gateway')
infoLog(`Sofie: Quantel gateway  Copyright (c) 2021 Norsk rikskringkasting AS (NRK)

Sofie: Quantel gateway comes with ABSOLUTELY NO WARRANTY.

This is free software, and you are welcome to redistribute it
under certain conditions (GPL v2.0 or later).
See https://github.com/nrkno/tv-automation-quantel-gateway/blob/master/LICENSE`)

let cliOpts = yargs
	.boolean('dummy')
	.number('port')
	.string('isa')
	.number('watchdog')
	.number('memory')
	.default('dummy', false)
	.default('port', 3000)
	.default('watchdog', 60)
	.default('memory', 0)
	.help()
	.usage('$0', 'Start the Sofie TV Automation Quantel Gateway')
	.describe('dummy', 'Modify behaviour to match ISA dummy server')
	.describe('port', 'Port number to listen on')
	.describe('isa', 'ISA endpoint (server[:port]) for initial connection (no http:)')
	.describe('watchdog', 'Interval (s) between watchdog checks. 0 for none.')
	.describe('memory', 'Internal (s) between memory logging. 0 to disable.')
	.argv

// console.log(cliOpts)

interface JSONError {
	status: number
	message: string
	stack: string
}

export const app = new Koa()
const router = new Router()
const instanceId = Math.floor(Math.random() * Number.MAX_SAFE_INTEGER).toString(16)
let currentStatus: ExternalStatus = 'OK'

app.use(async (ctx, next) => {
	const start = performance.now()
	await Promise.race([
		next(),
		new Promise<void>((resolve) => {
			setTimeout(() => {
				ctx.status = 524 // "A Timeout Occurred"
				ctx.body = {
					status: 524,
					message: `Quantel Gateway internal timeout after ${Math.floor(performance.now() - start)} ms.`
				}
				resolve()
			}, TIMEOUT_TIME)
			// Note: Quantel gw client will timeout in 5s
		})
	])
})

app.use(bodyParser({
	onerror: (err, ctx) => {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: err.message,
		 	stack: '' } as JSONError
	}
}))

router.get('/', async (ctx) => {
	ctx.body = await Quantel.listZones()
})

router.post('/connect/:addr', async (ctx) => {
	try {
		if (ctx.params.addr.indexOf(':') < 0) {
			ctx.params.addr += ':2096'
		}
		if (ctx.params.addr.indexOf(',') >= 0) {
			ctx.body = await Quantel.getISAReference(
				ctx.params.addr.split(',').map((x: string) => `http://${x}`))
		} else {
			ctx.body = await Quantel.getISAReference(`http://${ctx.params.addr}`)
		}
	} catch (err) {
		if (err.message.indexOf('ENOTFOUND') >= 0) {
			err.status = 404
			err.message = `Not found: ${err.message}`
		} else if (err.message.indexOf('ECONNREFUSED') >= 0) {
			err.status = 502
			err.message = `Bad gateway: ${err.message}`
		} else if (!(err instanceof Quantel.ConnectError)) {
			err.status = 500
		}
		throw err
	}
})

router.get('/connect', async (ctx) => {
	ctx.body = await Quantel.getConnectionDetails()
})

router.get('/health', async (ctx) => {
	if (Quantel.connectAttempted() === false) {
		ctx.body = {
			status: 'WARNING',
			name: 'Sofie Automation Quantel Gateway',
			updated: `${(new Date()).toISOString()}`,
			documentation: 'https://github.com/nrkno/tv-automation-quantel-gateway',
			version: '3',
			statusMessage: 'Waiting for connection request to Quantel server.',
			instanceId
		} as StatusResponse
	} else {
		ctx.body = {
			status: currentStatus,
			name: 'Sofie Automation Quantel Gateway',
			updated: `${(new Date()).toISOString()}`,
			documentation: 'https://github.com/nrkno/tv-automation-quantel-gateway',
			version: '3',
			statusMessage: currentStatus === 'FAIL' ?
				'Last response was a server error - check Kibana logs' :
				'Functioning as expected - last response was successful',
			instanceId
		} as StatusResponse
	}
})

router.post('/kill/me/if/you/are/sure', async (ctx) => {
	ctx.body = { status: 'Application shutting down in 1s!' }
	setTimeout(shutdown, 1000)
})
router.post('/debug/:debug', async (ctx) => {
	setDebug(ctx.params.debug === '1')
	ctx.body = { status: `Debug logging set to ${debug}` }
})
router.post('/timeout/:timeoutTime', async (ctx) => {
	const newTimeoutTime = parseInt(ctx.params.timeoutTime, 10)
	if (newTimeoutTime >= 0 && !Number.isNaN(newTimeoutTime)) {
		TIMEOUT_TIME = newTimeoutTime
		ctx.body = { status: `Timeout time set to ${debug}` }
	} else {
		ctx.body = { status: `Bad argument, /timeout/:timeoutTime timeoutTime must be a number!` }
	}
})

router.get('/:zoneID.json', async (ctx) => {
	if (ctx.params.zoneID.toLowerCase() === 'default') {
		ctx.body = await Quantel.getDefaultZoneInfo()
	} else {
		let zones = await Quantel.listZones()
		let inTheZone = zones.find(x => x.zoneNumber === +ctx.params.zoneID || x.zoneName === ctx.params.zoneID)
		if (inTheZone) {
			ctx.body = inTheZone
		} else {
			ctx.status = 404,
			ctx.body = {
				status: 404,
				message: `Not found. Could not find a zone called '${ctx.params.zoneID}'.`,
				stack: ''
			} as JSONError
		}
	}
})

// TODO consider adding support for other zone portals other than default

router.get('/:zoneID/', async (ctx) => {
	if (ctx.params.zoneID === 'default') {
		ctx.body = [ 'server/', 'clip/', 'format/', 'copy/' ]
	} else {
		let zones = await Quantel.listZones()
		let inTheZone = zones.find(z => z.zoneName === ctx.params.zoneID || z.zoneNumber.toString() === ctx.params.zoneID)
		if (inTheZone) {
			ctx.body = [ 'server/', 'clip/', 'format/', 'copy/' ]
		} else {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. Could not find a zone called '${ctx.params.zoneID}'.`,
				stack: ''
			} as JSONError
		}
	}
})

router.get('/default/format/', async (ctx) => {
	ctx.body = await Quantel.getFormats()
})

router.get('/default/format/:formatID', async (ctx) => {
	if (isNaN(+ctx.params.formatID) || +ctx.params.formatID < 0 || +ctx.params.formatID > 65535) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Format ID must be a non-negative short number.',
			stack: ''
		} as JSONError
		return
	}
	try {
		ctx.body = await Quantel.getFormatInfo({
			formatNumber: +ctx.params.formatID
		})
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A format with identifier '${ctx.params.formatID}' was not found.`,
				stack: ''
			}
		} else {
			throw err
		}
	}
})

router.get('/default/copy', async (ctx) => {
	ctx.body = await Quantel.getCopiesRemaining()
})

router.get('/default/copy/:copyID', async (ctx) => {
	if (isNaN(+ctx.params.copyID) || +ctx.params.copyID < 1) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Copy ID is a clip ID that must be a positive integer.',
			stack: ''
		} as JSONError
		return
	}
	try {
		ctx.body = await Quantel.getCopyRemaining({ clipID: +ctx.params.copyID })
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A copy associated with clip ID '${ctx.params.copyID}' was not found.`,
				stack: ''
			}
		} else {
			throw err
		}
	}
})

router.post('/default/copy', async (ctx) => {
	let clone: Quantel.CloneInfo = {} as Quantel.CloneInfo
	try {
		if (ctx.body && ctx.status === 400) return
		clone = ctx.request.body as Quantel.CloneInfo
		if (clone.zoneID && (isNaN(+clone.zoneID) || +clone.zoneID < 0)) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. Where present, a clone request must use a positive integer for the zone ID.',
				stack: ''
			}
			return
		}
		if (isNaN(+clone.clipID) || +clone.clipID < 1) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. A clone request must have a positive integer for the source clip ID.',
				stack: ''
			}
			return
		}
		if (isNaN(+clone.poolID) || +clone.poolID < 0) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. A clone request must have a positive integer for the destiniation pool ID.',
				stack: ''
			}
			return
		}
		if (clone.priority &&
				(isNaN(+clone.priority) || +clone.priority < 0 || +clone.priority > Quantel.Priority.HIGH)) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. A clone request with priority specified must use an integer in the range 0 to 15.',
				stack: ''
			}
			return
		}
		if (clone.history) {
			if (typeof (clone.history as any) === 'string') {
				if ((clone.history as any) === 'false') clone.history = false
				else if ((clone.history as any) === 'true') clone.history = true
			}
			if (typeof (clone.history as any) !== 'boolean') {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Bad request. A history parameter for a clone request must be either \'true\' or \'false\'.',
					stack: ''
				}
				return
			}
		}
		ctx.body = await Quantel.cloneInterZone(clone)
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. One of the source clip ID '${clone.clipID}', source zone ID '${clone.zoneID}' or destiniation pool ID '${clone.poolID}' was not found.`,
				stack: ''
			}
		} else {
			throw err
		}
	}
})

router.get('/default/server/', async (ctx) => {
	ctx.body = await Quantel.getServers()
})

router.get('/default/server/:serverID.json', async (ctx) => {
	let servers = await Quantel.getServers()
	let matchedServer = servers.find(x =>
		x.ident === +ctx.params.serverID || x.name === ctx.params.serverID)
	if (matchedServer) {
		ctx.body = matchedServer
	} else {
		ctx.status = 404
		ctx.body = {
			status: 404,
			message: `Not found. A server with identifier '${ctx.params.serverID}' was not found.`,
			stack: ''
		} as JSONError
	}
})

router.get('/default/server/:serverID/', async (ctx) => {
	let servers = await Quantel.getServers()
	let matchedServer = servers
		.find(x => x.ident === +ctx.params.serverID || x.name === ctx.params.serverID) // don't allow name resolution from here downwards
	if (matchedServer) {
		ctx.body = [ 'port/' ]
	} else {
		ctx.status = 404
		ctx.body = {
			status: 404,
			message: `Not found. A server with identifier '${ctx.params.serverID}' was not found.`,
			stack: ''
		} as JSONError
	}
})

router.get('/default/server/:serverID/port/', async (ctx) => {
	let servers = await Quantel.getServers()
	let matchedServer = servers
		.find(x => x.ident === +ctx.params.serverID || x.name === ctx.params.serverID)
	if (matchedServer) {
		ctx.body = matchedServer.portNames
	} else {
		ctx.status = 404
		ctx.body = {
			status: 404,
			message: `Not found. A server with identifier '${ctx.params.serverID}' was not found.`,
			stack: ''
		} as JSONError
	}
})

router.put('/default/server/:serverID/port/:portID/channel/:channelID', async (ctx) => {
	await queue.add(async () => {
		ctx.body = await Quantel.createPlayPort({
			serverID: ctx.params.serverID,
			portName: ctx.params.portID,
			channelNo: +ctx.params.channelID
		})
	}, { priority: 0 })
})

router.get('/default/server/:serverID/port/:portID', async (ctx) => {
	await queue.add(async () => {
		ctx.body = await Quantel.getPlayPortStatus({
			serverID: ctx.params.serverID,
			portName: ctx.params.portID
		})
	}, { priority: 0 })
})

router.post('/default/server/:serverID/port/:portID/reset', async (ctx) => {
	await queue.add(async () => {
		ctx.body = await Quantel.releasePort({
			serverID: ctx.params.serverID,
			portName: ctx.params.portID,
			resetOnly: true
		})
	}, { priority: 1 })
})

router.get('/default/server/:serverID/port/:portID/properties', async (ctx) => {
	await queue.add(async () => {
		ctx.body = await Quantel.getPortProperties({
			serverID: ctx.params.serverID,
			portName: ctx.params.portID
		})
	}, { priority: 0 })
})

router.delete('/default/server/:serverID/port/:portID', async (ctx) => {
	await queue.add(async () => {
		ctx.body = await Quantel.releasePort({
			serverID: ctx.params.serverID,
			portName: ctx.params.portID
		})
	}, { priority: 0 })
})

router.get('/default/clip', async (ctx) => {
	if (!ctx.query || Object.keys(ctx.query).length === 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Missing search query parameter, e.g. \'?Title=...\'',
			stack: ''
		} as JSONError
	} else {
		try {
			if (ctx.query.limit) {
				if (isNaN(+ctx.query.limit) || +ctx.query.limit < 1) {
					ctx.status = 400
					ctx.body = {
						status: 400,
						message: 'Limit parameter must be a positive number.',
						stack: ''
					}
					return
				}
			}
			ctx.body = await Quantel.searchClips({ ...ctx.query, limit: +ctx.query.limit })
		} catch (err) {
			if (err.message.indexOf('BadColumnData') >= 0) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: `Bad request. Unknown search parameter name '${Object.keys(ctx.query)}'.`,
					stack: ''
				} as JSONError
			} else {
				throw err
			}
		}
	}
})

router.get('/default/clip/:clipID', async (ctx) => {
	if (isNaN(+ctx.params.clipID) || +ctx.params.clipID < 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Clip ID must be a positive number.',
			stack: ''
		} as JSONError
		return
	}
	try {
		ctx.body = await Quantel.getClipData({
			clipID: +ctx.params.clipID
		})
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A clip with identifier '${ctx.params.clipID}' was not found.`,
				stack: ''
			}
		} else {
			throw err
		}
	}
})

router.delete('/default/clip/:clipID', async (ctx) => {
	if (isNaN(+ctx.params.clipID) || +ctx.params.clipID < 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Clip ID must be a positive number.',
			stack: ''
		} as JSONError
		return
	}
	try {
		ctx.body = {
			deleted: await Quantel.deleteClip({ clipID: +ctx.params.clipID })
		}
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A clip with identifier '${ctx.params.clipID}' was not found.`,
				stack: ''
			}
		} else {
			throw err
		}
	}

})

router.get('/default/clip/:clipID/fragments', async (ctx) => {
	if (isNaN(+ctx.params.clipID) || +ctx.params.clipID < 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Clip ID must be a positive number.',
			stack: ''
		} as JSONError
		return
	}
	try {
		let fragments = await Quantel.getFragments({
			clipID: +ctx.params.clipID
		})
		if (fragments.fragments.length > 0) {
			ctx.body = fragments
		} else {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A clip with identifier '${ctx.params.clipID}' was not found.`,
				stack: ''
			} as JSONError
		}
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A clip with identifier '${ctx.params.clipID}' was not found.`,
				stack: ''
			} as JSONError
		} else {
			throw err
		}
	}
})

router.get('/default/clip/:clipID/fragments/:in-:out', async (ctx) => {
	if (isNaN(+ctx.params.clipID) || +ctx.params.clipID < 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Clip ID must be a positive number.',
			stack: ''
		} as JSONError
		return
	}
	if (isNaN(+ctx.params.in) || +ctx.params.in < 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. In point must be a positive number.',
			stack: ''
		} as JSONError
		return
	}
	if (isNaN(+ctx.params.out) || +ctx.params.out < 0) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: 'Bad request. Out point must be a positive number.',
			stack: ''
		} as JSONError
		return
	}
	if (+ctx.params.out <= +ctx.params.in) {
		ctx.status = 400
		ctx.body = {
			status: 400,
			message: `Bad request. Out point "${ctx.params.out}" must be after in point "${ctx.params.in}".`,
			stack: ''
		} as JSONError
		return
	}
	try {
		let fragments = await Quantel.getFragments({
			clipID: +ctx.params.clipID,
			start: +ctx.params.in,
			finish: +ctx.params.out
		})
		if (fragments.fragments.length > 0) {
			ctx.body = fragments
		} else {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A clip with identifier '${ctx.params.clipID}' was not found or range is outside of clip boundary.`,
				stack: ''
			} as JSONError
		}
	} catch (err) {
		if (err.message.indexOf('BadIdent') >= 0) {
			ctx.status = 404
			ctx.body = {
				status: 404,
				message: `Not found. A clip with identifier '${ctx.params.clipID}' was not found.`,
				stack: ''
			} as JSONError
		} else {
			throw err
		}
	}
})

router.get('/default/server/:serverID/port/:portID/fragments/', async (ctx) => {
	try {
		let options: Quantel.PortFragmentRef = {
			serverID: ctx.params.serverID,
			portName: ctx.params.portID
		}
		if (ctx.query.start) {
			if (ctx.query.start && (isNaN(+ctx.query.start) || +ctx.query.start < 0)) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Get port fragments parameter \'start\' must be non-negative integer.',
					stack: ''
				} as JSONError
				return
			}
			options.start = +ctx.query.start
		}
		if (ctx.query.finish) {
			if (ctx.query.finish && (isNaN(+ctx.query.finish) || +ctx.query.finish < 0)) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Get port fragments parameter \'finish\' must be non-negative integer.',
					stack: ''
				} as JSONError
				return
			}
			options.finish = +ctx.query.finish
			if (options.start && options.finish + options.start > 0x7fffffff) {
				options.finish = 0x7fffffff - options.start
			}
			if (options.finish && options.start && options.finish <= options.start) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Get port fragments \'finish\' must be after \'start\'.',
					stack: ''
				} as JSONError
				return
			}
		}
		ctx.body = await Quantel.getPortFragments(options)
	} catch (err) {
		throw err
	}
})

router.post('/default/server/:serverID/port/:portID/fragments/', async (ctx) => {
	try {
		if (ctx.body && ctx.status === 400) return
		// Note: @types/koa-bodyparser has tried to included stricter typing .... but has forgotten arrays
		//       See https://github.com/DefinitelyTyped/DefinitelyTyped/pull/53715 and related
		let fragments: Quantel.ServerFragmentTypes[] = ctx.request.body as Quantel.ServerFragmentTypes[]
		if (!Array.isArray(fragments) || fragments.length === 0 || ctx.request.type !== 'application/json') {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. Fragments must be a JSON array with at least one element.',
				stack: ''} as JSONError
			return
		}
		if (ctx.query.offset && (isNaN(+ctx.query.offset) || +ctx.query.offset < 0)) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. Optional offset parameter must be a non-negative integer.',
				stack: ''
			} as JSONError
			return
		}
		ctx.body = await Quantel.loadPlayPort({
			serverID: ctx.params.serverID,
			portName: ctx.params.portID,
			fragments: fragments,
			offset: ctx.query.offset ? +ctx.query.offset : 0,
			dummy: cliOpts.dummy
		})
	} catch (err) {
		if (err.message.indexOf('was expected')) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'A parsing error prevented converting JSON representations of fragments into internal values.',
				stack: err.stack
			}
		} else {
			throw err
		}
	}
})

router.delete('/default/server/:serverID/port/:portID/fragments/', async (ctx) => {
	try {
		let options: Quantel.WipeInfo = {
			serverID: ctx.params.serverID,
			portName: ctx.params.portID
		}
		if (ctx.query.start) {
			if (ctx.query.start && (isNaN(+ctx.query.start) || +ctx.query.start < 0)) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Wipe parameter \'start\' must be non-negative integer.',
					stack: ''
				} as JSONError
				return
			}
			options.start = +ctx.query.start
		}
		if (ctx.query.frames) {
			if (ctx.query.frames === 'MAX') {
				ctx.query.frames = `${0x7fffffff}`
			}
			if (ctx.query.frames && (isNaN(+ctx.query.frames) || +ctx.query.frames < 0)) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Wipe parameter \'frames\' must be non-negative integer.',
					stack: ''
				} as JSONError
				return
			}
			options.frames = +ctx.query.frames
		} else {
			options.frames = 0x7fffffff
		}
		if (options.start && options.frames + options.start > 0x7fffffff) {
			options.frames = 0x7fffffff - options.start
		}
		ctx.body = await Quantel.wipe(options)
	} catch (err) {
		throw err
	}
})

router.post('/default/server/:serverID/port/:portID/trigger/:trigger', async (ctx) => {
	try {
		let trigger: Quantel.Trigger
		switch (ctx.params.trigger) {
			case 'START': trigger = Quantel.Trigger.START; break
			case 'STOP': trigger = Quantel.Trigger.STOP; break
			case 'JUMP': trigger = Quantel.Trigger.JUMP; break
			default:
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Trigger must have path parameter \'START\', \'STOP\' or \'JUMP\'.',
					stack: ''
				} as JSONError
				return
		}
		let options: Quantel.TriggerInfo = {
			serverID: ctx.params.serverID,
			portName: ctx.params.portID,
			trigger: trigger
		}
		if (ctx.query.offset) {
			if (ctx.query.offset && (isNaN(+ctx.query.offset) || +ctx.query.offset < 0)) {
				ctx.status = 400
				ctx.body = {
					status: 400,
					message: 'Bad request. Optional offset parameter must be a non-negative integer.',
					stack: ''
				} as JSONError
				return
			}
			options.offset = +ctx.query.offset
		}
		ctx.body = await Quantel.trigger(options)
	} catch (err) {
		throw err
	}
})

router.post('/default/server/:serverID/port/:portID/jump', async (ctx) => {
	try {
		if (ctx.query.offset && (isNaN(+ctx.query.offset) || +ctx.query.offset < 0)) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. Optional offset parameter must be a non-negative integer.',
				stack: ''
			} as JSONError
			return
		}
		let options: Quantel.JumpInfo = {
			serverID: ctx.params.serverID,
			portName: ctx.params.portID,
			offset : ctx.query.offset ? +ctx.query.offset : 0
		}
		ctx.body = await Quantel.jump(options)
	} catch (err) {
		throw err
	}
})

router.put('/default/server/:serverID/port/:portID/jump', async (ctx) => {
	try {
		if (ctx.query.offset && (isNaN(+ctx.query.offset) || +ctx.query.offset < 0)) {
			ctx.status = 400
			ctx.body = {
				status: 400,
				message: 'Bad request. Optional offset parameter must be a non-negative integer.',
				stack: ''
			} as JSONError
			return
		}
		let options: Quantel.JumpInfo = {
			serverID: ctx.params.serverID,
			portName: ctx.params.portID,
			offset : ctx.query.offset ? +ctx.query.offset : 0
		}
		ctx.body = await Quantel.setJump(options)
	} catch (err) {
		throw err
	}
})

app.use(async (ctx, next) => {
	try {
		debugLog(JSON.stringify({
			type: 'request',
			method: ctx.request.method,
			path: `${ctx.URL.pathname}${ctx.request.querystring ? `?${ctx.request.querystring}` : ''}`
		}))

		await next()
		if (ctx.status === 404) {
			if (!ctx.body) {
				ctx.body = {
					status: 404,
					message: `Not found. Request ${ctx.method} ${ctx.path}`,
					stack: ''
				} as JSONError
				ctx.status = 404
			}
		}
	} catch (err) {
		if (err.message.indexOf('TRANSIENT') >= 0) {
			err.message = 'Bad gateway. CORBA subsystem reports a transient connection problem. Check network connection and/or ISA server status.'
			err.statusCode = 502
		}
		ctx.status = err.statusCode || err.status || 500
		ctx.body = {
			status: ctx.status,
			message: err.message,
			stack: err.stack
		} as JSONError
	}
	if (ctx.status >= 400) {
		errorLog(JSON.stringify({
			type: ctx.status >= 500 ? 'server_error' : 'client_error',
			method: ctx.request.method,
			path: `${ctx.URL.pathname}${ctx.request.querystring ? `?${ctx.request.querystring}` : ''}`,
			status: ctx.status,
			message: ctx.body.message,
			stack: ctx.body.stack
		}))
	}
	if (ctx.status >= 500) {
		currentStatus = 'FAIL'
	}
	if (ctx.status < 500 && currentStatus === 'FAIL') {
		currentStatus = 'OK'
	}
})

app.use(router.routes())

let watchDogCalls = 0
let watchDogReceives = 0
if (cliOpts.watchdog > 0) {
	// Just a check to watch the watch dog:
	setInterval(() => {
		if (watchDogCalls < 10 || watchDogReceives < 10) {
			// that's weird, the watchdog should have been called by now
			errorLog(`Watchdog calls: ${watchDogCalls}, receives: ${watchDogReceives}, that's no good, shutting down!`)
			shutdown()
		} else {
			infoLog(`Watchdog calls: ${watchDogCalls}, receives: ${watchDogReceives}`)
			watchDogCalls = 0
			watchDogReceives = 0
		}
	}, 3600 * 1000) // called every hour
}
function watchDog (interval: number, count: number = 0) {
	setTimeout(() => {
		watchDogCalls++
		let req = get(`http://localhost:${cliOpts.port}/`, res => {
			watchDogReceives++
			res.on('error', err => {
				if (count === 2) {
					errorLog(`Watchdog error on response: ${err.message}. Shutting down to trigger auto-restart in 5s.`)
					setTimeout(shutdown, 5000)
					res.resume()
				} else {
					infoLog(`Watchdog error on response: ${err.message}. Incrementing counter to ${++count}/2.`)
					res.resume()
					watchDog(interval >> 1, count)
				}
			})
			if (res.statusCode && res.statusCode >= 400) {
				if (count === 2) {
					errorLog(`Watchdog failure with status ${res.statusCode}. Shutting down to trigger auto-restart in 5s.`)
					setTimeout(shutdown, 5000)
					res.resume()
				} else {
					infoLog(`Watchdog failure with status ${res.statusCode}. Incrementing counter to ${++count}/2.`)
					res.resume()
					watchDog(interval >> 1, count)
				}
			} else {
				res.resume()
				debugLog('Watchdog test successful.')
				watchDog(interval, 0)
			}
		})
		req.on('error', err => {
			errorLog(`Watchdog error on request: ${err.message}. Shutting down to trigger auto-restart in 5s.`)
			setTimeout(shutdown, 5000)
		})
	}, interval)
}

let server: Server

if (require.main === module) {
	server = app.listen(cliOpts.port)
	server.on('error', errorLog)
	server.on('listening', async () => {
		infoLog(`Quantel gateway HTTP API - server running on port ${cliOpts.port}`)
		if (cliOpts.isa) {
			let connectResult: Quantel.ConnectionDetails
			try {
				if (cliOpts.isa.indexOf(':') < 0) {
					cliOpts.isa += ':2096'
				}
				if (cliOpts.isa.indexOf(',') >= 0) {
					connectResult = await Quantel.getISAReference(
						cliOpts.isa.split(',').map((x: string) => `http://${x}`))
				} else {
					connectResult = await Quantel.getISAReference(`http://${cliOpts.isa}`)
				}
				console.dir(connectResult)
			} catch (err) {
				if (err.message.indexOf('ENOTFOUND') >= 0) {
					errorLog(`Not found: ${err.message}`)
				} else if (err.message.indexOf('ECONNREFUSED') >= 0) {
					errorLog(`Bad gateway: ${err.message}`)
				} else if (!(err instanceof Quantel.ConnectError)) {
					errorLog(`Connection error: ${err.message}`)
				}
			}
		}
		if (cliOpts.watchdog > 0) {
			infoLog(`Starting watchdog with interval ${cliOpts.watchdog}s`)
			watchDog(cliOpts.watchdog * 1000)
		} else {
			infoLog('Server is starting without a watchdog.')
		}
	})
	server.on('close', () => {
		Quantel.destroyOrb()
	})
}

function shutdown () {
	infoLog('Server shutdown starting...')

	try {
		// Try to close the server gracefully before shutting down:
		server.close((err) => {
			if (err) {
				errorLog('Error closing server: ', err)
				process.exit(42)
			} else {
				infoLog('Server shutdown completed.')
				process.exit(0)
			}
		})
		// Also add a timeout, in case the server has not closed within 3 seconds
		const WAIT_KILL_DURATION = 3000
		setTimeout(() => {
			errorLog(`Timeout after ${WAIT_KILL_DURATION}ms when trying to close server. Forcing exit.`)
			process.exit(43)
		}, WAIT_KILL_DURATION)
	} catch (err) {
		errorLog('Unknown error during shutdown: ' + err)
		process.exit(44)
	}
}

if (cliOpts.memory > 0) {
	setInterval(() => {
		// global.gc()
		infoLog(JSON.stringify(process.memoryUsage()))
	}, cliOpts.memory * 1000)
}

process.on('SIGINT', () => {
	infoLog('SIGINT received')
	shutdown()
})
process.on('SIGTERM', () => {
	infoLog('SIGTERM received')
	shutdown()
})
