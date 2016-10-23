/*
var minServer = new Server({port: 82});

minServer.on('connection', function(socket)
{
	alert('server got connection')

	socket.on('data', function(data)
	{
		socket.write('HTTP/1.1 200 OK\r\nContent-length: 5\r\n\r\nhello', function(){
			socket.close();
		})
	})
	
	socket.on('close', function(){
		
	})
});

/////////

var socket = new Socket();

socket.connect({port: 80, host: '103.18.110.230'}, function()
{
	alert('sock connected');
	
	socket.on('data', function(data)
	{
		alert(data);
	})
	
	socket.write('GET / HTTP/1.1\r\nHost: origami64.net\r\n\r\n')
})
*/
