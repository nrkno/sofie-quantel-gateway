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

import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Clip-level Quantel gateway tests for querying clips', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		await Quantel.getISAReference('http://localhost:2096')
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

	test('Delete a clip', async () => {
		await expect(Quantel.deleteClip({ clipID: 42 }))
		.resolves.toEqual(true)
		await expect(Quantel.deleteClip({ clipID: 43 }))
		.resolves.toEqual(false)
	})

	test('Attempt to delete a clip with unknown ID', async () => {
		await expect(Quantel.deleteClip({ clipID: 666 }))
		.rejects.toThrow('BadIdent')
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
