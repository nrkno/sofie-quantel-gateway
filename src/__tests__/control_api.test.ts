import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Control-level Quantel gateway tests', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
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
		await spawn.stop()
	})
})
