import * as Koa from 'koa'
import * as Router from 'koa-router'
import * as bodyParser from 'koa-bodyparser'
import { Quantel } from '.'

interface JSONError {
	status: number,
	message: string,
	stack: string,
}

export const app = new Koa()
const router = new Router()

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
		ctx.body = await Quantel.getISAReference(`http://${ctx.params.addr}`)
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
		ctx.body = [ 'server/', 'clip/' ]
	} else {
		let zones = await Quantel.listZones()
		let inTheZone = zones.find(z => z.zoneName === ctx.params.zoneID || z.zoneNumber.toString() === ctx.params.zoneID)
		if (inTheZone) {
			ctx.body = [ 'server/', 'clip/' ]
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
	ctx.body = await Quantel.createPlayPort({
		serverID: ctx.params.serverID,
		portName: ctx.params.portID,
		channelNo: +ctx.params.channelID
	})
})

router.get('/default/server/:serverID/port/:portID', async (ctx) => {
	ctx.body = await Quantel.getPlayPortStatus({
		serverID: ctx.params.serverID,
		portName: ctx.params.portID
	})
})

router.delete('/default/server/:serverID/port/:portID', async (ctx) => {
	ctx.body = await Quantel.releasePort({
		serverID: ctx.params.serverID,
		portName: ctx.params.portID
	})
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
		ctx.body = await Quantel.searchClips(ctx.query)
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
			message: 'Bad request. Out point must be after in point.',
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

router.post('/default/server/:serverID/port/:portID/fragments/', async (ctx) => {
	try {
		if (ctx.body && ctx.status === 400) return
		let fragments: Quantel.ServerFragment[] = ctx.request.body
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
			offset: ctx.query.offset ? +ctx.query.offset : 0
		})
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

// Make the default error handler use JSON
app.use(async (ctx, next) => {
	try {
		await next()
		if (ctx.status === 404) {
			if (!ctx.body) {
				ctx.body = {
					status: 404,
					message: `Not found. Request ${ctx.method} ${ctx.path}`,
					stack: ''
				} as JSONError
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
})

app.use(router.routes())

if (!module.parent) {
	app.listen(3000)
	console.log('Server running on port 3000')
}
