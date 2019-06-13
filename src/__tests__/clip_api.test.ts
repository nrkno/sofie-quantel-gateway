import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Test framework', () => {

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

	test('Search for clip by title', async () => {
		await expect(Quantel.searchClips({ Title: 'Once upon*' })).resolves.toMatchObject([{
			type: 'ClipDataSummary',
			ClipID: 2,
			CloneID: 2,
			Completed: new Date('2019-06-12T19:16:00.000Z'),
			Created: new Date('2019-06-12T19:16:00.000Z'),
			Description: 'This is the best programme ever to be produced.',
			Frames: '1234',
			Owner: 'Mine Hands Off',
			PoolID: 11,
			Title: 'Once upon a time in Quantel'
		}])
	})

	test('Search for non-existant clip by title', async () => {
		await expect(Quantel.searchClips({ Title: 'Once under*' })).resolves.toHaveLength(0)
	})

	afterAll(async () => {
		await spawn.stop()
	})
})
