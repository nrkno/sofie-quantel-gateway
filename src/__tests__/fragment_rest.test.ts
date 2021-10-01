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
import request from 'request-promise-native'

describe('Clip-fragments REST API tests', () => {

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

	test('Get fragments for clip', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2/fragments').then(JSON.parse))
		.resolves.toMatchObject({
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

	test('Attempt to get fragments for unknown clip', async () => {
		await expect(request.get('http://localhost:3000/default/clip/42/fragments'))
		.rejects.toThrow('A clip with identifier \'42\' was not found')
		await expect(request.get('http://localhost:3000/default/clip/42/fragments'))
		.rejects.toThrow('404')
	})

	test('Attempt to get fragments for unknown clip by name', async () => {
		await expect(request.get('http://localhost:3000/default/clip/whoami/fragments'))
		.rejects.toThrow('Clip ID must be a positive number')
		await expect(request.get('http://localhost:3000/default/clip/whoami/fragments'))
		.rejects.toThrow('400')
	})

	// In-out test

	test('Get fragments with range', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10-20').then(JSON.parse))
		.resolves.toMatchObject({
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

	test('Get fragments with bad range', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10-9'))
		.rejects.toThrow('must be after in point')
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10-9'))
		.rejects.toThrow('400')
	})

	test('Attempt to get fragments for unknown clip with range', async () => {
		await expect(request.get('http://localhost:3000/default/clip/42/fragments/10-20'))
		.rejects.toThrow('A clip with identifier \'42\' was not found')
		await expect(request.get('http://localhost:3000/default/clip/42/fragments/10-20'))
		.rejects.toThrow('404')
	})

	test('Get fragments with range, start-point only', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10'))
		.rejects.toThrow('Request') // does not route
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10'))
		.rejects.toThrow('404')
	})

	test('Get fragments with range, bad numbers', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10-wtf'))
		.rejects.toThrow('Out point must be a positive number') // does not route
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/10-wtf'))
		.rejects.toThrow('400')
	})

	test('Get fragments with range, bad numbers', async () => {
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/wtf-20'))
		.rejects.toThrow('In point must be a positive number') // does not route
		await expect(request.get('http://localhost:3000/default/clip/2/fragments/wtf-20'))
		.rejects.toThrow('400')
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
		await spawn.stop()
	})
})
