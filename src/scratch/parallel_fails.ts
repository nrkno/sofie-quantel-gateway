import { Quantel } from '../index'
import * as spawn from '../__tests__/spawn_server'

async function run () {
	let isaIOR: string
	isaIOR = await spawn.start()
	console.log(isaIOR)

	console.log(await Quantel.getISAReference('http://127.0.0.1:2096'))

	await spawn.stop()

	console.log((await Promise.all([
		Quantel.getServers().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x))])).toString())

	console.log((await Promise.all([
		Quantel.testConnection().then(console.log, x => Promise.resolve(x)),
		Quantel.testConnection().then(console.log, x => Promise.resolve(x)),
		Quantel.testConnection().then(console.log, x => Promise.resolve(x))])).toString())

	console.log((await Promise.all([
		Quantel.testConnection().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x))])).toString())

	console.log((await Promise.all([
		Quantel.testConnection().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x))])).toString())

	console.log((await Promise.all([
		Quantel.testConnection().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x))])).toString())

	console.log((await Promise.all([
		Quantel.testConnection().then(console.log, x => Promise.resolve(x)),
		Quantel.getServers().then(console.log, x => Promise.resolve(x))])).toString())

	console.log('DONE !!!!')

	/* test('Restart server and get details', async () => {
		isaIOR = await spawn.start()
		// await new Promise((resolve) => setTimeout(() => resolve(), 1000))
		await expect(Quantel.testConnection()).resolves.toBe('PONG!')
		await expect(Quantel.getISAReference()).resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://127.0.0.1:2096',
			isaIOR,
			refs: [ 'http://127.0.0.1:2096' ],
			robin: 2 } as Quantel.ConnectionDetails)
	})

	test('Check that get servers now works', async () => {
		await expect(Quantel.getServers()).resolves.toBeTruthy()
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	}) */
}

run()
