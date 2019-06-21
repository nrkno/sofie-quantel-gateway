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

describe('Port-level Quantel gateway tests', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		isaIOR = isaIOR
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('Create play port', async () => {
		await expect(Quantel.createPlayPort({
			serverID: 1100,
			portName: 'ginger',
			channelNo: 2 }))
			.resolves.toMatchObject({
				type: 'PortInfo',
 				assigned: true,
				audioOnly: false,
				channelNo: 2,
				portName: 'ginger',
				serverID: 1100 } as Quantel.PortInfo)
	})

	test('Get play port status', async () => {
		await expect(Quantel.getPlayPortStatus({
			serverID: 1100,
			portName: 'Port 1' })).resolves.toMatchObject({
				type: 'PortStatus',
				status: 'playing&readyToPlay',
				speed: 0.5,
				serverID: 1100,
				refTime: '10:11:12:13',
				portTime: '13:12:11:10',
				portName: 'Port 1',
				portID: 42,
				outputTime: '23:59:59:24',
				offset: 44,
				framesUnused: 43,
				endOfData: 45,
				channels: [ 1 ]
			} as Quantel.PortStatus)
	})

	test('Release port', async () => {
		await expect(Quantel.releasePort({
			serverID: 1100,
			portName: 'Port 1'
		})).resolves.toMatchObject({
			type: 'ReleaseStatus',
			serverID: 1100,
			portName: 'Port 1',
			released: true
		} as Quantel.ReleaseStatus)
	})

	test('Wipe entire port', async () => {
		await expect(Quantel.wipe({
			serverID: 1100,
			portName: 'Port 1'
		})).resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 0x7fffffff,
			serverID: 1100,
			start: 0,
			wiped: true
		} as Quantel.WipeResult)
	})

	test('Wipe a port between two points', async () => {
		await expect(Quantel.wipe({
			serverID: 1100,
			portName: 'Port 1',
			start: 123,
			frames: 321
		})).resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 321,
			start: 123,
			wiped: true
		})
	})

	test('Load play port with fragments', async () => {
		await expect(Quantel.loadPlayPort({
			serverID: 1100,
			portName: 'Port 1',
			fragments: [{
				type: 'VideoFragment',
				start: 0,
				finish: 1000,
				format: 90,
				poolID: 11,
				rushFrame: 543210,
				poolFrame: 42,
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
				poolFrame: 43,
				rushID: 'fedcba98765432100123456789abcdef',
				skew: 24,
				trackNum: 0
			} as Quantel.AudioFragment]})).resolves.toEqual({
				type: 'PortLoadStatus',
				serverID: 1100,
				portName: 'Port 1',
				offset: 0,
				fragmentCount: 2
			} as Quantel.PortLoadStatus)
	})

	test('Get play port fragments', async () => {
		await expect(Quantel.getPortFragments({
			serverID: 1100,
			portName: 'Port 1'
		})).resolves.toMatchObject({
			type: 'ServerFragments',
			serverID: 1100,
			portName: 'Port 1',
			clipID: -1,
			fragments: [{
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
			} as Quantel.AudioFragment]} as Quantel.PortServerFragments)
	})

	afterAll(async () => {
		await spawn.stop()
	})
})
