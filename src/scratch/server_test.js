const http = require('http')

app = http.createServer((_request, response) => {
	response.writeHead(200, { 'Content-Type': 'text/plain' })
	response.write(isaIOR)
	response.end()
})
app.on('error', () => { console.log('Nope!', +process.argv[2]) })
app.on('listening', () => {
	console.log('Hearing you', process.argv[2]); app.close() })
app.listen(+process.argv[2])
