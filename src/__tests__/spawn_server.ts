import { fork, ChildProcess } from 'child_process'
import * as http from 'http'

let mockServer: ChildProcess | null = null
let app: http.Server | null = null

export async function start (): Promise<string> {
	let resolved = false
	return new Promise((resolve, reject) => {
		mockServer = fork(`src/__tests__/test_server.js`, { stdio: 'pipe' } as any)
		let isaIOR = ''

		mockServer.stdout.on('data', (data) => {
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
			app.on('listening', () => {
				resolved = true
				resolve(isaIOR.trim())
			})
			app.on('error', e => {
				if (!resolved) {
					reject(e)
				} else {
					console.error(e)
				}
			})
		})

		mockServer.stderr.on('data', (data) => {
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

export async function stop (): Promise<undefined> {
	return new Promise((resolve) => {
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
