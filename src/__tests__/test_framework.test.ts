import { Quantel } from '../index'
import * as spawn from './spawn_server'
import { app } from '../server'
import { Server } from 'http'
import * as request from 'request-promise-native'

describe('Test framework', () => {

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

	test('Default get connection reference', async () => {
		await expect(Quantel.getISAReference()).resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://localhost:2096',
			isaIOR } as Quantel.ConnectionDetails)
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('Test HTTP API is running', async () => {
		await expect(request.post('http://localhost:3000/connect/127.0.0.1')).resolves.toBeTruthy()
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
