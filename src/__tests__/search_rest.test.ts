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
import { app } from '../server'
import { Server } from 'http'
import * as request from 'request-promise-native'

describe('Search REST API tests', () => {

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
			isaIOR,
		 	refs: [ 'http://127.0.0.1:2096' ],
			robin: 0 } as Quantel.ConnectionDetails)
	})

	test('Search for clip by title', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*').then(JSON.parse))
		.resolves.toMatchObject([{
			type: 'ClipDataSummary',
			ClipID: 2,
			ClipGUID: 'e977435806f24b37aed871bf15a2eef9',
			CloneId: 2,
			Completed: '2019-06-12T19:16:00.000Z',
			Created: '2019-06-12T19:16:00.000Z',
			Description: 'This is the best programme ever to be produced.',
			Frames: '1234',
			Owner: 'Mine Hands Off',
			PoolID: 11,
			Title: 'Once upon a time in Quantel'
		}])
	})

	test('Search for clip by GUID', async () => {
		await expect(request.get('http://localhost:3000/default/clip?ClipGUID=e977435806f24b37aed871bf15a2eef9').then(JSON.parse))
		.resolves.toMatchObject([{
			type: 'ClipDataSummary',
			ClipID: 2,
			ClipGUID: 'e977435806f24b37aed871bf15a2eef9',
			CloneId: 2,
			Completed: '2019-06-12T19:16:00.000Z',
			Created: '2019-06-12T19:16:00.000Z',
			Description: 'This is the best programme ever to be produced.',
			Frames: '1234',
			Owner: 'Mine Hands Off',
			PoolID: 11,
			Title: 'Once upon a time in Quantel'
		}])
	})

	test('Search for clip with no results', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=WhoAmI').then(JSON.parse))
		.resolves.toHaveLength(0)
	})

	test('Search for clip with bad parameter', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Fred=Ginger'))
		.rejects.toThrow('Unknown search parameter name \'Fred\'')
		await expect(request.get('http://localhost:3000/default/clip?Fred=Ginger'))
		.rejects.toThrow('400')
	})

	test('Search for clip with 2 parameters', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&PoolID=11').then(JSON.parse))
		.resolves.toMatchObject([{
			type: 'ClipDataSummary',
			ClipID: 2,
			ClipGUID: 'e977435806f24b37aed871bf15a2eef9',
			CloneId: 2,
			Completed: '2019-06-12T19:16:00.000Z',
			Created: '2019-06-12T19:16:00.000Z',
			Description: 'This is the best programme ever to be produced.',
			Frames: '1234',
			Owner: 'Mine Hands Off',
			PoolID: 11,
			Title: 'Once upon a time in Quantel'
		}])
	})

	test('Search for clip with no paramters', async () => {
		await expect(request.get('http://localhost:3000/default/clip'))
		.rejects.toThrow('Missing search query parameter')
		await expect(request.get('http://localhost:3000/default/clip'))
		.rejects.toThrow('400')
	})

	test('Search for clip, id only', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&idOnly').then(JSON.parse))
		.resolves.toEqual([ 2 ])
	})

	test('Search for clip by title with limit', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&limit=1').then(JSON.parse))
		.resolves.toMatchObject([{
			type: 'ClipDataSummary',
			ClipID: 2,
			ClipGUID: 'e977435806f24b37aed871bf15a2eef9',
			CloneId: 2,
			Completed: '2019-06-12T19:16:00.000Z',
			Created: '2019-06-12T19:16:00.000Z',
			Description: 'This is the best programme ever to be produced.',
			Frames: '1234',
			Owner: 'Mine Hands Off',
			PoolID: 11,
			Title: 'Once upon a time in Quantel'
		}])
	})

	test('Expect search fail if limit is a string', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&limit=wtf'))
		.rejects.toThrow('Limit parameter must be a positive number')
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&limit=wtf'))
		.rejects.toThrow('400')
	})

	test('Expect search fail if limit is a zero', async () => {
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&limit=wtf'))
		.rejects.toThrow('Limit parameter must be a positive number')
		await expect(request.get('http://localhost:3000/default/clip?Title=Once upon*&limit=wtf'))
		.rejects.toThrow('400')
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
