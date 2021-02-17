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

const quantel = require('../../build/Release/quantel_gateway')

// const SegfaultHandler = require('segfault-handler')
// SegfaultHandler.registerHandler('crash2.log')



// process.on('SIGHUP', () => {
// 	quantel.closeServer()
// 	console.log("Close server called.")
// 	setTimeout(() => {}, 3000)
// })

quantel.runServer()

const loopy = setInterval(() => {
	// console.log('loopy');
	quantel.performWork();
}, 10);

const closeDown = () => {
	console.error("closeDown called");
	clearInterval(loopy);
	console.log('quantel.closeServer about to call');
	quantel.closeServer();
	console.log('quantel.closeServer returned');
	// quantel.performWork();
	// quantel.deactivatePman();
	setTimeout(() => { process.exit(); }, 3000);
}

process.on('SIGINT', closeDown)
process.on('message', m => {
	if (typeof m === 'string' && m === 'close') {
		closeDown();
	}
})

process.on('exit', () => {
	console.log('BANG!!!')
})

