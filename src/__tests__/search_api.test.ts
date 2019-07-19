/*
	 Copyright (c) 2019 Norsk rikskringkasting AS (NRK)

	 This file is part of Sofie: The Modern TV News Studio Automation
	 System (Quantel gateway)

	 This program is free software; you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation; either version 2 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License along
	 with this program; if not, write to the Free Software Foundation, Inc.,
	 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

import * as Quantel from '../index'
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
			ClipGUID: 'e977435806f24b37aed871bf15a2eef9',
			CloneId: 2,
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

	test('Search for clip, return clipID only', async () => {
		await expect(Quantel.searchClips({ Title: 'Once upon*', idOnly: '' }))
		.resolves.toEqual([ 2 ])
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
