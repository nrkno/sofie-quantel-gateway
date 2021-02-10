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

import { fork, ChildProcess } from 'child_process'
import * as http from 'http'

let mockServer: ChildProcess | null = null
let app: http.Server | null = null

export async function start (): Promise<string> {
	let resolved = false
	return new Promise((resolve, reject) => {
		mockServer = fork(`src/__tests__/test_server.js`, { stdio: 'pipe' } as any)
		let isaIOR = ''

		mockServer.stdout!.on('data', (data) => {
			if (resolved) {
				console.log(data.toString())
				return
			}
			isaIOR = data.toString().trim()
			app = http.createServer((_request, response) => {
				response.writeHead(200, { 'Content-Type': 'text/plain' })
				response.write(isaIOR)
				response.end()
			})
			app.listen(2096)
			app.on('listening', (e: Error) => {
				console.log('Test server listening OK', e)
				resolved = true
				resolve(isaIOR.trim())
			})
			app.on('error', e => {
				if (!resolved) {
					console.error(e)
					reject(e)
				} else {
					console.error(e)
				}
			})
		})

		mockServer.stderr!.on('data', (data) => {
			console.error(data.toString())
		})

		mockServer.on('error', e => {
			if (!resolved) {
				reject(e)
			} else {
				console.error(e)
			}
		})
	})
}

export async function stop (): Promise<void> {
	return new Promise<void>((resolve) => {
		if (mockServer) {
			mockServer.kill()
			mockServer.on('exit', (_code, _signal) => {
				if (app) {
					app.close(() => {
						resolve()
					})
				} else {
					resolve()
				}
			})
		} else {
			if (app) app.close(() => resolve())
			else { resolve() }
		}
	})
}
