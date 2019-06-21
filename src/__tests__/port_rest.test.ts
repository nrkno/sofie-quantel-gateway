import { Quantel } from '../index'
import * as spawn from './spawn_server'
import { app } from '../server'
import { Server } from 'http'
import * as request from 'request-promise-native'

describe('Port-level REST API tests', () => {

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
			isaIOR } as Quantel.ConnectionDetails)
	})

	test('Create port, server by number', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/2').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'PortInfo',
			assigned: true,
			audioOnly: false,
			channelNo: 2,
			portName: 'ginger',
			serverID: 1100 })
	})

	test('Create port, server by name', async () => {
		await expect(request.put('http://localhost:3000/default/server/Server 1100/port/ginger/channel/2').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'PortInfo',
			assigned: true,
			audioOnly: false,
			channelNo: 2,
			portName: 'ginger',
			serverID: 1100 })
	})

	test('Attempt to create port with bad server', async () => {
		await expect(request.put('http://localhost:3000/default/server/1101/port/ginger/channel/2'))
		.rejects.toThrow('Could not find a server with identifier \'1101\'')
		await expect(request.put('http://localhost:3000/default/server/1101/port/ginger/channel/2'))
		.rejects.toThrow('404')
	})

	test('Attempt to create port that already exists', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/channel/2'))
		.rejects.toThrow('Port \'Port 1\' on server \'1100\' is already assigned to channels \'[1]\' and cannot be assigned to channel \'2\'')
		await expect(request.put('http://localhost:3000/default/server/1100/port/Port 1/channel/2'))
		.rejects.toThrow('409')
	})

	test('Attempt to create a port on a channel that is already in use', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/1'))
		.rejects.toThrow('Cannot assign channel \'1\' to port \'ginger\' on server \'1100\' as it is already assigned to port \'Port 2\'')
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/1'))
		.rejects.toThrow('400')
	})

	test('Attempt to create a port on a channel that does not exist', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/4'))
		.rejects.toThrow('Channel number of \'4\' exceeds maximum index for server of \'3\'')
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/4'))
		.rejects.toThrow('400')
	})

	test('Attempt to create a port on a channel given by name', async () => {
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/wtf'))
		.rejects.toThrow('Channel number must be a non-negative integer')
		await expect(request.put('http://localhost:3000/default/server/1100/port/ginger/channel/4'))
		.rejects.toThrow('400')
	})

	test('Get play port status, server by number', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1').then(JSON.parse))
		.resolves.toMatchObject({
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
		})
	})

	test('Get play port status, server by number', async () => {
		await expect(request.get('http://localhost:3000/default/server/Server 1100/port/Port 1').then(JSON.parse))
		.resolves.toMatchObject({
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
		})
	})

	test('Attempt to get play port status, bad server', async () => {
		await expect(request.get('http://localhost:3000/default/server/1101/port/Port 1'))
		.rejects.toThrow('Could not find a server with identifier \'1101\'')
		await expect(request.get('http://localhost:3000/default/server/1101/port/Port 1'))
		.rejects.toThrow('404')
	})

	test('Attempt to get play port status, bad port name', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/whoami'))
		.rejects.toThrow('Could not find a port called \'whoami\' on server \'Server 1100\'')
		await expect(request.get('http://localhost:3000/default/server/1100/port/whoami'))
		.rejects.toThrow('404')
	})

	test('Release port with server by number', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'ReleaseStatus',
			serverID: 1100,
			portName: 'Port 1',
			released: true
		})
	})

	test('Release port with server by name', async () => {
		await expect(request.delete('http://localhost:3000/default/server/Server 1100/port/Port 1').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'ReleaseStatus',
			serverID: 1100,
			portName: 'Port 1',
			released: true
		})
	})

	test('Attempt to release port with bad server name', async () => {
		await expect(request.delete('http://localhost:3000/default/server/Sewage 1100/port/Port 1'))
		.rejects.toThrow('Could not find a server with identifier \'Sewage 1100\'')
		await expect(request.delete('http://localhost:3000/default/server/Sewage 1100/port/Port 1'))
		.rejects.toThrow('404')
	})

	test('Attempt to release port with bad port name', async () => {
		await expect(request.delete('http://localhost:3000/default/server/Server 1100/port/whoami'))
		.rejects.toThrow('Could not find a port called \'whoami\' on server \'Server 1100\'')
		await expect(request.delete('http://localhost:3000/default/server/Server 1100/port/whoami'))
		.rejects.toThrow('404')
	})

	test('Get play port fragments with server number', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments').then(JSON.parse))
		.resolves.toMatchObject({
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
			}] })
	})

	test('Get play port fragments with server name', async () => {
		await expect(request.get('http://localhost:3000/default/server/Server 1100/port/Port 1/fragments').then(JSON.parse))
		.resolves.toMatchObject({
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
			}] })
	})

	test('Attempt to get play port fragments with bad server ident', async () => {
		await expect(request.get('http://localhost:3000/default/server/1099/port/Port 1/fragments'))
		.rejects.toThrow('Could not find a server with identifier \'1099\'')
		await expect(request.get('http://localhost:3000/default/server/1099/port/Port 1/fragments'))
		.rejects.toThrow('404')
	})

	test('Attempt to get play port fragments with bad port name', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/whoami/fragments'))
		.rejects.toThrow('Could not find a port called \'whoami\' on server \'Server 1100\'')
		await expect(request.get('http://localhost:3000/default/server/1100/port/whoami/fragments'))
		.rejects.toThrow('404')
	})

	test('Get play port fragments with range', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=10&finish=20').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'ServerFragments',
			serverID: 1100,
			portName: 'Port 1',
			clipID: -1,
			fragments: [{
				type: 'VideoFragment',
				start: 10,
				finish: 20,
				format: 90,
				poolID: 11,
				rushFrame: 543210,
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
				rushFrame: 123456,
				poolFrame: 321,
				rushID: 'fedcba98765432100123456789abcdef',
				skew: 24,
				trackNum: 0
			}] })
	})

	test('Attempt to get play port fragments with bad start', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=wtf'))
		.rejects.toThrow('Get port fragments parameter \'start\' must be non-negative integer')
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=wtf'))
		.rejects.toThrow('400')
	})

	test('Attempt to get play port fragments with negative start', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=-1'))
		.rejects.toThrow('Get port fragments parameter \'start\' must be non-negative integer')
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=-1'))
		.rejects.toThrow('400')
	})

	test('Attempt to get play port fragments with bad finish', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?finish=wtf'))
		.rejects.toThrow('Get port fragments parameter \'finish\' must be non-negative integer')
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?finish=wtf'))
		.rejects.toThrow('400')
	})

	test('Attempt to get play port fragments with negative finish', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?finish=-1'))
		.rejects.toThrow('Get port fragments parameter \'finish\' must be non-negative integer')
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?finish=-1'))
		.rejects.toThrow('400')
	})

	test('Attempt to get play port fragments with bad range', async () => {
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=10&finish=10'))
		.rejects.toThrow('Get port fragments \'finish\' must be after \'start\'')
		await expect(request.get('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=10&finish=10'))
		.rejects.toThrow('400')
	})

	test('Load play port fragments with server number', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/server/1100/port/Port 1/fragments',
			body: [{
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
			}],
			json: true }))
		.resolves.toMatchObject({
			type: 'PortLoadStatus',
			serverID: 1100,
			portName: 'Port 1',
			offset: 0,
			fragmentCount: 2
		})
	})

	test('Load play port fragments with server name', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/server/Server 1100/port/Port 1/fragments',
			body: [{
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
			}],
			json: true }))
		.resolves.toMatchObject({
			type: 'PortLoadStatus',
			serverID: 1100,
			portName: 'Port 1',
			offset: 0,
			fragmentCount: 2
		})
	})

	test('Attempt to load play port fragments with bad server', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/server/1142/port/Port 1/fragments',
			body: [{
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
			}],
			json: true }))
		.rejects.toThrow('Could not find a server with identifier \'1142\'')
	})

	test('Attempt to load play port fragments bad port name', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/server/1100/port/whoami/fragments',
			body: [{
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
			}],
			json: true }))
		.rejects.toThrow('Could not find a port called \'whoami\' on server \'Server 1100\'')
	})

	test('Attempt to load play port fragments bad fragments', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/server/1100/port/Port 1/fragments',
			body: {
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
			},
			json: true }))
		.rejects.toThrow('Fragments must be a JSON array with at least one element')
	})

	test('Attempt to load play port fragments empty fragments', async () => {
		await expect(request({
			method: 'POST',
			uri: 'http://localhost:3000/default/server/1100/port/Port 1/fragments',
			body: [{}, {}],
			json: true }))
		.rejects.toThrow('A parsing error prevented converting JSON representations of fragments into internal values')
	})

	test('Wipe all fragments from a port with server number', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 0x7fffffff,
			serverID: 1100,
			start: 0,
			wiped: true
		})
	})

	test('Wipe all fragments from a port with server name', async () => {
		await expect(request.delete('http://localhost:3000/default/server/Server 1100/port/Port 1/fragments').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 0x7fffffff,
			serverID: 1100,
			start: 0,
			wiped: true
		})
	})

	test('Attempt to wipe all fragments with bad server name', async () => {
		await expect(request.delete('http://localhost:3000/default/server/Sewage 1100/port/Port 1/fragments'))
		.rejects.toThrow('Could not find a server with identifier \'Sewage 1100\'')
		await expect(request.delete('http://localhost:3000/default/server/Sewage 1100/port/Port 1/fragments'))
		.rejects.toThrow('404')
	})

	test('Attempt to wipe all fragments with bad port name', async () => {
		await expect(request.delete('http://localhost:3000/default/server/Server 1100/port/whoami/fragments'))
		.rejects.toThrow('Could not find a port called \'whoami\' on server \'Server 1100\'')
		await expect(request.delete('http://localhost:3000/default/server/Server 1100/port/whoami/fragments'))
		.rejects.toThrow('404')
	})

	test('Wipe fragments with a range', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=123&frames=321').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 321,
			serverID: 1100,
			start: 123,
			wiped: true
		})
	})

	test('Wipe fragments with a range, start only', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=123').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 0x7fffffff - 123,
			serverID: 1100,
			start: 123,
			wiped: true
		})
	})

	test('Wipe fragments with a range, finish set to MAX', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=123&frames=MAX').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 0x7fffffff - 123,
			serverID: 1100,
			start: 123,
			wiped: true
		})
	})

	test('Wipe fragments with a range, finish only', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?frames=321').then(JSON.parse))
		.resolves.toMatchObject({
			type: 'WipeResult',
			portName: 'Port 1',
			frames: 321,
			serverID: 1100,
			start: 0,
			wiped: true
		})
	})

	test('Attempt to wipe fragments with a range and bad start', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=wtf').then(JSON.parse))
		.rejects.toThrow('Wipe parameter \'start\' must be non-negative integer.')
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=wtf').then(JSON.parse))
		.rejects.toThrow('400')
	})

	test('Attempt to wipe fragments with a range and negative start', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=-1').then(JSON.parse))
		.rejects.toThrow('Wipe parameter \'start\' must be non-negative integer.')
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?start=-1').then(JSON.parse))
		.rejects.toThrow('400')
	})

	test('Attempt to wipe fragments with a range and bad frames', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?frames=wtf').then(JSON.parse))
		.rejects.toThrow('Wipe parameter \'frames\' must be non-negative integer.')
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?frames=wtf').then(JSON.parse))
		.rejects.toThrow('400')
	})

	test('Attempt to wipe fragments with a range and negative frames', async () => {
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?frames=-1').then(JSON.parse))
		.rejects.toThrow('Wipe parameter \'frames\' must be non-negative integer.')
		await expect(request.delete('http://localhost:3000/default/server/1100/port/Port 1/fragments?frames=-1').then(JSON.parse))
		.rejects.toThrow('400')
	})

	afterAll(async () => {
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
