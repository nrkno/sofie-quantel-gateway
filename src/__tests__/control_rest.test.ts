import { Quantel } from '../index'
import * as spawn from './spawn_server'
import { app } from '../server'
import { Server } from 'http'
import * as request from 'request-promise-native'

describe('Control-level REST API tests', () => {

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

	test('START playing using server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/START').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true
		})
	})

	test('STOP playing using server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/STOP?offset=123').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 123
		})
	})

	test('JUMP now', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/JUMP').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true
		})
	})

	test('Hard jump', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=101').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'HardJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 101
		})
	})

	test('Soft jump', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=101').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggeredJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: 101
		})
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
