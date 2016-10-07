var server = new Server({port: 81});

server.on('connection', function(client)
{	
	//alert('got client');

	client.on('data', function(data)
	{
		alert(data);
		
		client.write(
			'HTTP/1.1 200 OK\r\n'+
			'Content-type: text/html\r\n'+
			'Content-length: 37\r\n' +
			'\r\n' +
			'http server in project64 what is life',
			function(){
				alert('finished write');
			}
		);
	});
});