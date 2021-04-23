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
import { app } from '../server'
import { Server } from 'http'
import * as request from 'request-promise-native'

const aBriefPause = (p: number) => new Promise((resolve) => {
	setTimeout(resolve, p)
})

describe('Copy-level REST API tests', () => {

	let isaIOR: string
	let server: Server

	beforeAll(async () => {
		isaIOR = await spawn.start()
		await new Promise<void>((resolve, reject) => {
			server = app.listen(3000) // TODO change this to a config parameter
			server.on('listening', () => {
				resolve()
			})
			server.on('error', e => {
				reject(e)
			})
		})
	})

	test('Test connect', async () => {
		await expect(request.post('http://localhost:3000/connect/127.0.0.1').then(JSON.parse))
		.resolves.toStrictEqual({
			type: 'ConnectionDetails',
			href: 'http://127.0.0.1:2096',
			isaIOR,
		 	refs: [ 'http://127.0.0.1:2096' ],
			robin: 0 } as Quantel.ConnectionDetails)
	})

	test('Clone clip inter-zone with history', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/copy',
			body: {
				zoneID: 1000,
				clipID: 1234,
				poolID: 4321
			} as Quantel.CloneInfo,
			json: true }))
		.resolves.toMatchObject({
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

	test('Clone clip inter-zone without history', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/copy',
			body: {
				zoneID: 1000,
				clipID: 1234,
				poolID: 4321,
				history: false
			} as Quantel.CloneInfo,
			json: true }))
		.resolves.toMatchObject({
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
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/copy',
			body: {
				clipID: 1234,
				poolID: 4321,
				priority: 15
			} as Quantel.CloneInfo,
			json: true }))
		.resolves.toMatchObject({
			type: 'CloneResult',
			clipID: 1234,
			poolID: 4321,
			priority: 15,
			history: true,
			copyCreated: true,
			copyID: 423
		} as Quantel.CloneResult)
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await new Promise<void>((resolve, reject) => {
			server.close(e => {
				if (e) {
					reject(e)
				} else { resolve() }
			})
		})
		await aBriefPause(500)
		await spawn.stop()
	})
})
