import { Quantel } from '../index'
import * as spawn from './spawn_server'

// jest.mock('../../build/Release/quantel_gateway')
// const q = require('bindings')('quantel_gateway')
describe('All together now', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
	})

	test('Default get connection reference', async () => {
		await expect(Quantel.getISAReference()).resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://localhost:2096',
			isaIOR } as Quantel.ConnectionDetails)
	})

	test('Test connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	afterAll(async () => {
		await spawn.stop()
	})
})
