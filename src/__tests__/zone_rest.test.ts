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

describe('Zone-level REST API tests', () => {

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

	test('Zone information', async () => {
		await expect(request.get('http://localhost:3000/').then(JSON.parse))
		.resolves.toMatchObject([{
			type: 'ZonePortal',
			zoneNumber: 1000,
			zoneName: 'Zone 1000',
			isRemote: false
		} as Quantel.ZoneInfo, {
			type: 'ZonePortal',
			zoneNumber: 2000,
			zoneName: 'Zone 2000',
			isRemote: true
		} as Quantel.ZoneInfo ])
	})

	test('Zone information, zone by number', async () => {
		await expect(request.get('http://localhost:3000/1000.json').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'ZonePortal',
			zoneNumber: 1000,
			zoneName: 'Zone 1000',
			isRemote: false
		})
	})

	test('Zone information, zone by name', async () => {
		await expect(request.get('http://localhost:3000/Zone 2000.json').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'ZonePortal',
			zoneNumber: 2000,
			zoneName: 'Zone 2000',
			isRemote: true
		} as Quantel.ZoneInfo)
	})

	test('Get zone unknown', async () => {
		await expect(request.get('http://localhost:3000/whoami.json'))
		.rejects.toThrow('Could not find a zone called \'whoami\'')
		await expect(request.get('http://localhost:3000/whoami.json'))
		.rejects.toThrow('404')
	})

	test('Get zone paths', async () => {
		await expect(request.get('http://localhost:3000/default/').then(JSON.parse))
		.resolves.toMatchObject([ 'server/', 'clip/', 'format/', 'copy/' ])
	})

	test('Get servers', async () => {
		await expect(request.get('http://localhost:3000/default/server/').then(JSON.parse))
		.resolves.toMatchObject([{
			type: 'Server',
			ident: 1100,
			down: false,
			name: 'Server 1100',
			numChannels: 4,
			pools: [ 11 ],
			portNames: [ 'Port 1', 'Port 2' ],
			chanPorts: [ 'Port 1', 'Port 2', '', '' ] },
		{ type: 'Server',
			ident: 1200,
			down: true,
			name: 'Server 1200',
			numChannels: 2,
			pools: [ 12 ],
			portNames: [ 'Port 1' ],
			chanPorts: [ 'Port 1', '' ] },
		{ type: 'Server',
			ident: 1300,
			down: false,
			name: 'Server 1300',
			numChannels: 3,
			pools: [ 13 ],
			portNames: [ 'Port 1', 'Port 2' ],
			chanPorts: [ 'Port 1', 'Port 2', '' ] } ])
	})

	test('Get server by identifier', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100.json').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'Server',
			ident: 1100,
			down: false,
			name: 'Server 1100',
			numChannels: 4,
			pools: [ 11 ],
			portNames: [ 'Port 1', 'Port 2' ],
			chanPorts: [ 'Port 1', 'Port 2', '', '' ]
		})
	})

	test('Get server by name', async () => {
		await expect(request.get('http://localhost:3000/default/server/Server 1100.json').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'Server',
			ident: 1100,
			down: false,
			name: 'Server 1100',
			numChannels: 4,
			pools: [ 11 ],
			portNames: [ 'Port 1', 'Port 2' ],
			chanPorts: [ 'Port 1', 'Port 2', '', '' ]
		})
	})

	test('Get server unknown', async () => {
		await expect(request.get('http://localhost:3000/default/server/whoami.json'))
		.rejects.toThrow('A server with identifier \'whoami\' was not found')
		await expect(request.get('http://localhost:3000/default/server/whoami.json'))
		.rejects.toThrow('404')
	})

	test('Get a format', async () => {
		await expect(request.get('http://localhost:3000/default/format/90').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'FormatInfo',
			formatNumber: 90,
			frameRate: 25,
			height: 576,
			width: 720,
			samples: 0,
			compressionName: 'Mpeg-2',
			formatName: 'Legacy 9E Mpeg 40 576i25',
			layoutName: '720x576i25'
		})
	})

	test('Attempt to get a non-existant format', async () => {
		await expect(request.get('http://localhost:3000/default/format/42'))
		.rejects.toThrow('A format with identifier \'42\' was not found')
		await expect(request.get('http://localhost:3000/default/format/42'))
		.rejects.toThrow('404')
	})

	test('Attempt to get a format by name', async () => {
		await expect(request.get('http://localhost:3000/default/format/whoami'))
		.rejects.toThrow('Format ID must be a non-negative short number')
		await expect(request.get('http://localhost:3000/default/format/whoami'))
		.rejects.toThrow('400')
	})

	test('Attempt to get a format by negative number', async () => {
		await expect(request.get('http://localhost:3000/default/format/-1'))
		.rejects.toThrow('Format ID must be a non-negative short number')
		await expect(request.get('http://localhost:3000/default/format/-1'))
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
