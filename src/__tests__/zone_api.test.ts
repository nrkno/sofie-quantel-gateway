import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Test framework', () => {

	let isaIOR: string

	beforeAll(async () => {
		isaIOR = await spawn.start()
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
		await expect(Quantel.getServers()).resolves.toHaveLength(3)
	})

	afterAll(async () => {
		await spawn.stop()
	})
})
