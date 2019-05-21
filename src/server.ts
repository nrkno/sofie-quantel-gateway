import * as Koa from 'koa'
import * as Router from 'koa-router'
import { Quantel } from '.'

const app = new Koa()
const router = new Router()

router.get('/', async (ctx) => {
	ctx.body = await Quantel.getZoneInfo()
})

router.get('/default.json', async (ctx) => {
	ctx.body = await Quantel.getZoneInfo()
})

router.get('/default/', async (ctx) => {
	ctx.body = await Quantel.getServers()
})

router.get('/default/:serverID.json', async (ctx) => {
	let servers = await Quantel.getServers()
	ctx.body = servers.find(x => x.ident === +ctx.params.serverID || x.name === ctx.params.serverID)
})

app.use(router.routes())

app.listen(3000)

console.log('Server running on port 3000')
