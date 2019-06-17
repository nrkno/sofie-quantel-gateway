import { Quantel } from '../index'
import * as spawn from './spawn_server'

describe('Zone-level Quantel gateway tests', () => {

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
		await spawn.stop()
	})
})
