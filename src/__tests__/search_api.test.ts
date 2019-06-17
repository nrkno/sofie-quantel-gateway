import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Clip-level Quantel gateway tests for searching', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		isaIOR = isaIOR
	})

	test('Search for clip by title', async () => {
		await expect(Quantel.searchClips({ Title: 'Once upon*' })).resolves.toHaveLength(1)
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
