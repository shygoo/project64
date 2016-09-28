function processData(data)
{
	var floats = new Float32Array(data);
	alert(floats[0] + " " +
	      floats[1] + " " +
	      floats[2]
	);
}

var server = new Server();
server.listen(8081);

server.on('connection', function(client)
{
	client.on('data', function(data)
	{
		if(data.byteLength == 12)
		{
			processData(data);
		}
	});
});
