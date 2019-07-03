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

describe('Clip-level REST API tests', () => {

	let isaIOR: string
	let server: Server

	beforeAll(async () => {
		isaIOR = await spawn.start()
		await new Promise((resolve, reject) => {
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
			isaIOR } as Quantel.ConnectionDetails)
	})

	test('Retrieve a clip that exists', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2').then(JSON.parse))
		.resolves.toEqual({
			ClipID: 2,
			Created: '2019-06-12T19:16:00.000Z',
			PlaceHolder: false,
			Title: 'Once upon a time in Quantel',
			type: 'ClipData'
		})
	})

	test('Attempt to retrieve a clip that does not exist', async () => {
		await expect(request.get('http://localhost:3000/default/clip/42'))
		.rejects.toThrow('A clip with identifier \'42\' was not found')
		await expect(request.get('http://localhost:3000/default/clip/42'))
		.rejects.toThrow('404')
	})

	test('Attempt to retrieve a clip with a name', async () => {
		await expect(request.get('http://localhost:3000/default/clip/whoami'))
		.rejects.toThrow('Clip ID must be a positive number')
		await expect(request.get('http://localhost:3000/default/clip/whoami'))
		.rejects.toThrow('400')
	})

	test('Delete a clip', async () => {
		await expect(request.delete('http://localhost:3000/default/clip/42').then(JSON.parse))
		.resolves.toMatchObject({ deleted: true })
		await expect(request.delete('http://localhost:3000/default/clip/43').then(JSON.parse))
		.resolves.toMatchObject({ deleted: false })
	})

	test('Attempt to delete a clip with unknown ID', async () => {
		await expect(request.delete('http://localhost:3000/default/clip/666'))
		.rejects.toThrow('A clip with identifier \'666\' was not found')
		await expect(request.delete('http://localhost:3000/default/clip/666'))
		.rejects.toThrow('404')
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await new Promise((resolve, reject) => {
			server.close(e => {
				if (e) {
					reject(e)
				} else { resolve() }
			})
		})
		await spawn.stop()
	})
})
