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

describe('Zone-level Quantel gateway tests', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		Quantel.getISAReference('http://localhost:2096')
		isaIOR = isaIOR
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('Get default zone info', async () => {
		await expect(Quantel.getDefaultZoneInfo()).resolves.toMatchObject({
			type: 'ZonePortal',
			zoneNumber: 1000,
			zoneName: 'Zone 1000',
			isRemote: false } as Quantel.ZoneInfo)
	})

	test('List zones', async () => {
		await expect(Quantel.listZones()).resolves.toMatchObject([{
			type: 'ZonePortal',
			zoneNumber: 1000,
			zoneName: 'Zone 1000',
			isRemote: false
		}, {
			type: 'ZonePortal',
			zoneNumber: 2000,
			zoneName: 'Zone 2000',
			isRemote: true } ] as Quantel.ZoneInfo[])
	})

	test('Get servers', async () => {
		// console.log(await Quantel.getServers())
		await expect(Quantel.getServers()).resolves.toMatchObject([ {
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

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
