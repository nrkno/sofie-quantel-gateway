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

describe('Async operations Quantel gateway tests', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		isaIOR = isaIOR
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('Try 2 requests at once', async () => {
		await expect(Promise.all([
			Quantel.getServers(),
			Quantel.getDefaultZoneInfo()
		])).resolves.toBeTruthy()
	})

	test('Try 3 requests at once', async () => {
		await expect(Promise.all([
			Quantel.getServers(),
			Quantel.getDefaultZoneInfo(),
			Quantel.testConnection()
		])).resolves.toBeTruthy()
	})

	test('Try 4 requests at once', async () => {
		await expect(Promise.all([
			Quantel.getServers(),
			Quantel.getDefaultZoneInfo(),
			Quantel.testConnection(),
			Quantel.getFormatInfo({ formatNumber: 90 })
		])).resolves.toBeTruthy()
	})

	test('Destory orb and try again', async () => {
		Quantel.destroyOrb()
		await expect(Promise.all([
			Quantel.getServers(),
			Quantel.getDefaultZoneInfo(),
			Quantel.testConnection(),
			Quantel.getFormatInfo({ formatNumber: 90 })
		])).resolves.toBeTruthy()
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
