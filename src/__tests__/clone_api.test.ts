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

describe('Copy-level Quantel gateway tests for cloning clips', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		isaIOR = isaIOR
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('Clone inter zone with history', async () => {
		await expect(Quantel.cloneInterZone({
			zoneID: 1000,
			clipID: 1234,
			poolID: 4321
		})).resolves.toMatchObject({
			type: 'CloneResult',
			zoneID: 1000,
			clipID: 1234,
			poolID: 4321,
			priority: 8,
			history: true,
			copyCreated: true,
			copyID: 421
		} as Quantel.CloneResult)
	})

	test('Clone inter zone without history', async () => {
		await expect(Quantel.cloneInterZone({
			zoneID: 1000,
			clipID: 1234,
			poolID: 4321,
			history: false
		})).resolves.toMatchObject({
			type: 'CloneResult',
			zoneID: 1000,
			clipID: 1234,
			poolID: 4321,
			priority: 8,
			history: false,
			copyCreated: true,
			copyID: 422
		} as Quantel.CloneResult)
	})

	test('Clone within zone', async () => {
		await expect(Quantel.cloneInterZone({
			clipID: 1234,
			poolID: 4321,
			priority: Quantel.Priority.HIGH
		})).resolves.toMatchObject({
			type: 'CloneResult',
			clipID: 1234,
			poolID: 4321,
			priority: Quantel.Priority.HIGH,
			history: true,
			copyCreated: true,
			copyID: 423
		} as Quantel.CloneResult)
	})

	test('Clone within zone, no copy', async () => {
		await expect(Quantel.cloneInterZone({
			clipID: 43,
			poolID: 4321,
			priority: Quantel.Priority.HIGH
		})).resolves.toMatchObject({
			type: 'CloneResult',
			clipID: 43,
			poolID: 4321,
			priority: Quantel.Priority.HIGH,
			history: true,
			copyCreated: false,
			copyID: 423
		} as Quantel.CloneResult)
	})

	test('Attempt to clone, bad clip ID', async () => {
		await expect(Quantel.cloneInterZone({
			zoneID: 1000,
			clipID: 666,
			poolID: 4321
		})).rejects.toThrow('BadIdent')
	})

	test('Copy remaining', async () => {
		await expect(Quantel.getCopyRemaining({ clipID: 42 }))
		.resolves.toMatchObject({
			type: 'CopyProgress',
			clipID: 42,
			protonsLeft: 128,
			totalProtons: 256,
			secsLeft: 19,
			priority: 9,
			ticketed: false
		} as Quantel.CopyProgress)
	})

	test('Attempt to get copy progress, non-existant copy', async () => {
		await expect(Quantel.getCopyRemaining({ clipID: 666 }))
		.rejects.toThrow('BadIdent')
	})

	test('Copies remaining', async () => {
		await expect(Quantel.getCopiesRemaining())
		.resolves.toEqual([{
			type: 'CopyProgress',
			clipID: 42,
			protonsLeft: 128,
			totalProtons: 256,
			secsLeft: -3,
			priority: 9,
			ticketed: false
		}] as Quantel.CopyProgress[])
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
