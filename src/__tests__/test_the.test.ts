import { Quantel } from '../index'

jest.mock('../../build/Release/quantel_gateway')
const q = require('bindings')('quantel_gateway')

test('Basic test', async () => {
	try {
		expect(Quantel.timecodeFromBCD(9)).toThrow()
	} catch (e) { }
	expect(q.timecodeFromBCD).toHaveBeenCalled()
	expect(Quantel.Trigger.START).toBe(0)
})
