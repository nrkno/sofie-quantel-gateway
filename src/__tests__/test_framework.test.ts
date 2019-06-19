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

	test('Bad route', async () => {
		await expect(request.get('http://localhost:3000/default/wrong', { json: true }).then(x => x, x => JSON.parse(x.message.slice(6))))
		.resolves.toEqual({
			status: 404,
			message: 'Not found. Request GET /default/wrong',
			stack: ''
		})
		await expect(request.get('http://localhost:3000/default/wrong'))
		.rejects.toThrow('Not found. Request GET /default/wrong')
	})

	test('Convert BCD timecode to string', () => {
		expect(Quantel.timecodeFromBCD(0x10111213)).toBe('10:11:12:13')
		expect(Quantel.timecodeFromBCD(0)).toBe('00:00:00:00')
		expect(Quantel.timecodeFromBCD(0x23595924)).toBe('23:59:59:24')
		expect(Quantel.timecodeFromBCD(0x23595929 | 0x40000000)).toBe('23:59:59;29')
	})

	test('Convert string timecode to BCD', () => {
		expect(Quantel.timecodeToBCD('10:11:12:13')).toBe(0x10111213)
		expect(Quantel.timecodeToBCD('00:00:00:00')).toBe(0)
		expect(Quantel.timecodeToBCD('23:59:59:24')).toBe(0x23595924)
		expect(Quantel.timecodeToBCD('23:59:59;29')).toBe(0x23595929 | 0x40000000)
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

describe('Error handling when no server running', () => {

	test('Test failed IOR HTTP connection', async () => {
		expect.assertions(1)
		await expect(Quantel.testConnection()).rejects.toThrow('CORBA subsystem: TRANSIENT')
	})

	test('Test fail to get servers 1', async () => {
		expect.assertions(1)
		await expect(Quantel.getServers()).rejects.toThrow('ECONNREFUSED')
	})
})

describe('Error handling when server has failed', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
	})

	test('Default get connection reference and close', async () => {
		await expect(Quantel.getISAReference()).resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://127.0.0.1:2096',
			isaIOR } as Quantel.ConnectionDetails)
	})

	test('Stopping server', async () => {
		await expect(spawn.stop()).resolves.toBeUndefined()
	})

	test('Test fail to get servers 1', async () => {
		expect.assertions(1)
		await expect(Quantel.getServers()).rejects.toThrow('CORBA subsystem: TRANSIENT')
	})

	test('Test the failed CORBA connection', async () => {
		expect.assertions(1)
		await expect(Quantel.testConnection()).rejects.toThrow('ECONNREFUSED')
	})

	test('Restart server and get details', async () => {
		isaIOR = await spawn.start()
		// await new Promise((resolve) => setTimeout(() => resolve(), 1000))
		await expect(Quantel.testConnection()).resolves.toBe('PONG!')
		await expect(Quantel.getISAReference()).resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://127.0.0.1:2096',
			isaIOR } as Quantel.ConnectionDetails)
	})

	test('Check that get servers now works', async () => {
		await expect(Quantel.getServers()).resolves.toBeTruthy()
	})

	afterAll(async () => {
		await spawn.stop()
	})
})
