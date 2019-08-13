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

describe('Control-level Quantel gateway tests', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
		Quantel.getISAReference('http://localhost:2096')
		isaIOR = isaIOR
	})

	test('Test CORBA connection', async () => {
		await expect(Quantel.testConnection()).resolves.toEqual('PONG!')
	})

	test('START playing', async () => {
		await expect(Quantel.trigger({
			serverID: 1100,
			portName: 'Port 1',
			trigger: Quantel.Trigger.START
		})).resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: undefined,
			success: true
		} as Quantel.TriggerResult)
	})

	test('STOP playing at offset', async () => {
		await expect(Quantel.trigger({
			serverID: 1100,
			portName: 'Port 1',
			trigger: Quantel.Trigger.START,
			offset: 123
		})).resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: 123,
			success: true
		} as Quantel.TriggerResult)
	})

	test('JUMP now', async () => {
		await expect(Quantel.trigger({
			serverID: 1100,
			portName: 'Port 1',
			trigger: Quantel.Trigger.JUMP
		})).resolves.toMatchObject({
			type: 'TriggerResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: undefined,
			success: true
		} as Quantel.TriggerResult)
	})

	test('Hard jump', async () => {
		await expect(Quantel.jump({
			serverID: 1100,
			portName: 'Port 1',
			offset: 101
		})).resolves.toMatchObject({
			type: 'HardJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			success: true,
			offset: 101
		} as Quantel.JumpResult)
	})

	test('Soft jump', async () => {
		await expect(Quantel.setJump({
			serverID: 1100,
			portName: 'Port 1',
			offset: 101
		})).resolves.toMatchObject({
			type: 'TriggeredJumpResult',
			serverID: 1100,
			portName: 'Port 1',
			offset: 101
		} as Quantel.JumpResult)
	})

	afterAll(async () => {
		Quantel.destroyOrb()
		await spawn.stop()
	})
})
