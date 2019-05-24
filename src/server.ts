import * as Koa from 'koa'
import * as Router from 'koa-router'
import { Quantel } from '.'

const app = new Koa()
const router = new Router()

router.get('/', async (ctx) => {
	ctx.body = await Quantel.listZones()
})

router.post('/connect/:addr', async (ctx) => {
	ctx.body = await Quantel.getISAReference(`http://${ctx.params.addr}`)
})

router.get('/:zoneID.json', async (ctx) => {
	console.log('Here I am', ctx.params)
	if (ctx.params.zoneID.toLowerCase() === 'default') {
		ctx.body = await Quantel.getDefaultZoneInfo()
	} else {
		let zones = await Quantel.listZones()
		ctx.body = zones.find(x => x.zoneNumber === +ctx.params.zoneID || x.zoneName === ctx.params.zoneID)
	}
})

// TODO consider adding support for other zone portals other than default

router.get('/:zoneID/', async (ctx) => {
	ctx.body = [ 'server/', 'clip/' ]
})

router.get('/default/server/', async (ctx) => {
	ctx.body = await Quantel.getServers()
})

router.get('/default/server/:serverID.json', async (ctx) => {
	let servers = await Quantel.getServers()
	ctx.body = servers.find(x => x.ident === +ctx.params.serverID || x.name === ctx.params.serverID)
})

router.get('/default/server/:serverID/', async (ctx) => {
	ctx.body = [ 'port/' ]
})

router.get('/default/server/:serverID/port/', async (ctx) => {
	let servers = await Quantel.getServers()
	let matchedServer = servers
		.find(x => x.ident === +ctx.params.serverID || x.name === ctx.params.serverID)
	ctx.body = matchedServer ? matchedServer.portNames : []
})

router.put('/default/server/:serverID/port/:portID/channel/:channelID', async (ctx) => {
	ctx.body = await Quantel.createPlayPort({
		serverID: +ctx.params.serverID,
		portName: ctx.params.portID,
		channelNo: +ctx.params.channelID
	})
})

router.get('/default/server/:serverID/port/:portID', async (ctx) => {
	ctx.body = await Quantel.getPlayPortStatus({
		serverID: +ctx.params.serverID,
		portName: ctx.params.portID
	})
})

router.delete('/default/server/:serverID/port/:portID', async (ctx) => {
	ctx.body = await Quantel.releasePort({
		serverID: +ctx.params.serverID,
		portName: ctx.params.portID
	})
})

router.get('/default/clip', async (ctx) => {
	if (!ctx.query || Object.keys(ctx.query).length === 0) {
		ctx.body = []
	} else {
		ctx.body = await Quantel.searchClips(ctx.query)
	}
})

router.get('/default/clip/:clipID', async (ctx) => {
	ctx.body = await Quantel.getClipData({
		clipID: +ctx.params.clipID
	})
})

router.get('/default/clip/:clipID/fragments', async (ctx) => {
	ctx.body = await Quantel.getFragments({
		clipID: +ctx.params.clipID
	})
})

router.get('/default/clip/:clipID/fragments/:in-:out', async (ctx) => {
	ctx.body = await Quantel.getFragments({
		clipID: +ctx.params.clipID,
		start: +ctx.params.in,
		finish: +ctx.params.out
	})
})

app.use(router.routes())

app.listen(3000)

console.log('Server running on port 3000')
