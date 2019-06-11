import { Quantel } from '../index'
import * as spawn from './spawn_server'
import { app } from '../server'
import { Server } from 'http'
import * as request from 'request-promise-native'

describe('T', () => {

	let isaIOR: string
	let server: Server

	beforeAll(async () => {
		isaIOR = await spawn.start()
		await new Promise((resolve, reject) => {
			server = app.listen(3000) // TODO change this to a config parameter
			server.on('listening', () => {
				resolve()
			})
			server.on('error', e => {
				reject(e)
			})
		})
	})

	test('Test connect', async () => {
		await expect(request.post('http://localhost:3000/connect/127.0.0.1').then(JSON.parse))
		.resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://127.0.0.1:2096',
			isaIOR } as Quantel.ConnectionDetails)
	})

	afterAll(async () => {
		await new Promise((resolve, reject) => {
			server.close(e => {
				if (e) {
					reject(e)
				} else { resolve() }
			})
		})
		await spawn.stop()
	})
})
