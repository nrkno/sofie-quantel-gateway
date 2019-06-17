import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Clip-level Quantel gateway tests for querying clips', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		isaIOR = isaIOR
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('Retrieve clip data with an ID', async () => {
		await expect(Quantel.getClipData({ clipID: 2 })).resolves.toMatchObject({
			type: 'ClipData',
			ClipID: 2,
			Created: new Date('2019-06-12T19:16:00.000Z'),
			PlaceHolder: false,
			Title: 'Once upon a time in Quantel'
		})
	})

	test('Fail to retrieve data if if clip not found', async () => {
		expect.assertions(1)
		await expect(Quantel.getClipData({ clipID: 42 })).rejects.toThrow('BadIdent')
	})

	afterAll(async () => {
		await spawn.stop()
	})
})
