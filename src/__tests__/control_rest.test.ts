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

describe('Control-level REST API tests', () => {

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

	test('START playing using server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/START').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true
		})
	})

	test('START playing using server name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/START').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true
		})
	})

	test('STOP playing using server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/STOP?offset=123').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 123
		})
	})

	test('STOP playing using server name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/STOP?offset=123').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 123
		})
	})

	test('JUMP now using server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/JUMP').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true
		})
	})

	test('JUMP now using server name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/JUMP').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true
		})
	})

	test('Attempt to trigger with an unknown trigger type', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/JUMPY'))
		.rejects.toThrow('Trigger must have path parameter \'START\', \'STOP\' or \'JUMP\'')
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/trigger/JUMPY'))
		.rejects.toThrow('400')
	})

	test('Attempt to trigger with a bad server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1234/port/Port 1/trigger/START'))
		.rejects.toThrow('Could not find a server with identifier \'1234\'')
		await expect(request.post('http://localhost:3000/default/server/1234/port/Port 1/trigger/START'))
		.rejects.toThrow('404')
	})

	test('Attempt to trigger with a bad server name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Smelly 1100/port/Port 1/trigger/START'))
		.rejects.toThrow('Could not find a server with identifier \'Smelly 1100\'')
		await expect(request.post('http://localhost:3000/default/server/Smelly 1100/port/Port 1/trigger/START'))
		.rejects.toThrow('404')
	})

	test('Attempt to trigger with a bad port name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/portal/trigger/START'))
		.rejects.toThrow('Could not find a port called \'portal\' on server \'Server 1100\'')
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/portal/trigger/START'))
		.rejects.toThrow('404')
	})

	test('Attempt to trigger with a non-integer offset', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/START?offset=wtf'))
		.rejects.toThrow('Optional offset parameter must be a non-negative integer.')
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/START?offset=wtf'))
		.rejects.toThrow('400')
	})

	test('Attempt to trigger with a negative offset', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/START?offset=-1'))
		.rejects.toThrow('Optional offset parameter must be a non-negative integer.')
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/trigger/START?offset=-1'))
		.rejects.toThrow('400')
	})

	test('Hard jump with server number', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=101').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'HardJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 101
		})
	})

	test('Hard jump with server name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Server 1100/port/Port 1/jump?offset=101').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'HardJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 101
		})
	})

	test('Attempt hard jump with bad server ident', async () => {
		await expect(request.post('http://localhost:3000/default/server/1987/port/Port 1/jump?offset=123'))
		.rejects.toThrow('Could not find a server with identifier \'1987\'')
		await expect(request.post('http://localhost:3000/default/server/1987/port/Port 1/jump?offset=123'))
		.rejects.toThrow('404')
	})

	test('Attempt hard jump with bad server name', async () => {
		await expect(request.post('http://localhost:3000/default/server/Smelly 1100/port/Port 1/jump?offset=123'))
		.rejects.toThrow('Could not find a server with identifier \'Smelly 1100\'')
		await expect(request.post('http://localhost:3000/default/server/Smelly 1100/port/Port 1/jump?offset=123'))
		.rejects.toThrow('404')
	})

	test('Attempt hard jump with bad port name', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/portal/jump?offset=123'))
		.rejects.toThrow('Could not find a port called \'portal\' on server \'Server 1100\'')
		await expect(request.post('http://localhost:3000/default/server/1100/port/portal/jump?offset=123'))
		.rejects.toThrow('404')
	})

	test('Attempt hard jump with non-number offset', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=wtf'))
		.rejects.toThrow('Optional offset parameter must be a non-negative integer')
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=wtf'))
		.rejects.toThrow('400')
	})

	test('Attempt hard jump with negative offset', async () => {
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=-1'))
		.rejects.toThrow('Optional offset parameter must be a non-negative integer')
		await expect(request.post('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=-1'))
		.rejects.toThrow('400')
	})

	test('Soft jump with server number', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=101').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggeredJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: 101
		})
	})

	test('Soft jump with server name', async () => {
		await expect(request.put('http://localhost:3000/default/server/Server 1100/port/Port 1/jump?offset=101').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'TriggeredJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: 101
		})
	})

	test('Attempt soft jump with bad server ident', async () => {
		await expect(request.put('http://localhost:3000/default/server/1123/port/Port 1/jump?offset=321'))
		.rejects.toThrow('Could not find a server with identifier \'1123\'')
		await expect(request.put('http://localhost:3000/default/server/1123/port/Port 1/jump?offset=321'))
		.rejects.toThrow('404')
	})

	test('Attempt soft jump with bad server name', async () => {
		await expect(request.put('http://localhost:3000/default/server/Smelly 1100/port/Port 1/jump?offset=321'))
		.rejects.toThrow('Could not find a server with identifier \'Smelly 1100\'')
		await expect(request.put('http://localhost:3000/default/server/Smelly 1100/port/Port 1/jump?offset=321'))
		.rejects.toThrow('404')
	})

	test('Attempt soft jump with bad port name', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/portal/jump?offset=321'))
		.rejects.toThrow('Could not find a port called \'portal\' on server \'Server 1100\'')
		await expect(request.put('http://localhost:3000/default/server/1100/port/portal/jump?offset=321'))
		.rejects.toThrow('404')
	})

	test('Attempt soft jump with non-number offset', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=wtf'))
		.rejects.toThrow('Optional offset parameter must be a non-negative integer')
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=wtf'))
		.rejects.toThrow('400')
	})

	test('Attempt soft jump with negative offset', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=-1'))
		.rejects.toThrow('Optional offset parameter must be a non-negative integer')
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/jump?offset=-1'))
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
