import { spawn, ChildProcess } from 'child_process'
import * as http from 'http'

let mockServer: ChildProcess | null = null
let app: http.Server | null = null

export async function start (): Promise<string> {
	let resolved = false
	return new Promise((resolve, reject) => {
		mockServer = spawn('cmd.exe', ['/c', 'ts-node.cmd', __dirname + '/test_server.ts' ], { shell: false })
		let isaIOR = ''

		mockServer.stdout.on('data', (data) => {
			isaIOR = data.toString()
			app = http.createServer((_request, response) => {
				response.writeHead(200, { 'Content-Type': 'text/plain' })
				response.write(isaIOR)
				response.end()
			})
			app.listen(2096)
			app.on('listening', () => {
				resolved = true
				resolve(isaIOR)
			})
			app.on('error', e => {
				if (!resolved) reject(e)
			})
		})

		mockServer.on('error', e => {
			if (!resolved) reject(e)
		})
	})
}

export async function stop (): Promise<undefined> {
	return new Promise((resolve) => {
		if (mockServer) {
			mockServer.kill('SIGTERM')
			mockServer.on('exit', () => {
				if (app) app.close(() => resolve())
			})
		} else {
			if (app) app.close(() => resolve())
		}
	})
}

start().then(console.log, console.error)
