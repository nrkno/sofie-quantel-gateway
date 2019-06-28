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

describe('Clip-level Quantel gateway tests for fragments', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		isaIOR = isaIOR
	})

	test('Get all fragments for a clip', async () => {
		await expect(Quantel.getFragments({ clipID: 2 })).resolves.toMatchObject({
			type: 'ServerFragments',
			clipID: 2,
			fragments: [ {
				type: 'VideoFragment',
				start: 0,
				finish: 1000,
				format: 90,
				poolID: 11,
				rushFrame: 543210,
				poolFrame: 123,
				rushID: '0123456789abcdeffedcba9876543210',
				skew: 42,
				trackNum: 0
			} as Quantel.VideoFragment, {
				type: 'AudioFragment',
				start: 0,
				finish: 1000,
				format: 73,
				poolID: 11,
				rushFrame: 123456,
				poolFrame: 321,
				rushID: 'fedcba98765432100123456789abcdef',
				skew: 24,
				trackNum: 0
			} as Quantel.AudioFragment ]
		})
	})

	test('Get fragments for a non-existant clip', async () => {
		expect.assertions(1)
		await expect(Quantel.getFragments({ clipID: 42 })).rejects.toThrow('BadIdent')
	})

	test('Get fragments with range', async () => {
		await expect(Quantel.getFragments({ clipID: 2, start: 10, finish: 20 })).resolves.toMatchObject({
			type: 'ServerFragments',
			clipID: 2,
			fragments: [ {
				type: 'VideoFragment',
				start: 10,
				finish: 20,
				format: 90,
				poolID: 11,
				rushFrame: 543345,
				poolFrame: 123,
				rushID: '0123456789abcdeffedcba9876543210',
				skew: 42,
				trackNum: 0
			} as Quantel.VideoFragment, {
				type: 'AudioFragment',
				start: 10,
				finish: 20,
				format: 73,
				poolID: 11,
				rushFrame: 123654,
				poolFrame: 321,
				rushID: 'fedcba98765432100123456789abcdef',
				skew: 24,
				trackNum: 0
			} as Quantel.AudioFragment ]
		})
	})

	test('Get fragments with range non-existant clip', async () => {
		expect.assertions(1)
		await expect(Quantel.getFragments({ clipID: 42, start: 10, finish: 20 })).rejects.toThrow('BadIdent')
	})

	// CORBA API does no inverse range check - gateway now does one
	test('Get fragments with bad range', async () => {
		expect.assertions(1)
		await expect(Quantel.getFragments({ clipID: 2, start: 20, finish: 10 })).rejects.toThrow('cannot be before')
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
