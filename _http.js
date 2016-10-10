var server = new Server({port: 80});

server.on('connection', function(client)
{
	var AWAITING_HEADER = 0;
	var AWAITING_BODY = 1;
	
	var state = AWAITING_HEADER;
	
	client.on('data', function(data)
	{
		var lines = data.toString().split(/\n/);
		var firstLine = lines[0].split(" ");
		var method = firstLine[0];
		var path = firstLine[1];
		var version = firstLine[2];
		var querydata = path.split('?', 2);
		if(querydata[1])
		{
			querydata = querydata[1].split('&');
		}
		var query = {};
		if(querydata.length)
		{
			for(var i in querydata)
			{
				var spl = querydata[i].split('=');
				query[spl[0]] = spl[1];
			}
		}
		
		var headers = {};
		
		for(var i = 1; lines[i].trim() != ''; i++)
		{
			var header = lines[i].split(': ', 2);
			headers[header[0]] = header[1].trim();
		}
		
		var body = {
			method: method,
			path: path,
			query: query,
			headers: headers
		};

		var bodyText = JSON.stringify(body);
		
		var response =
			'HTTP/1.1 200 OK\r\n'+
			'Content-type: application/json\r\n'+
			'Content-length: ' + bodyText.length + '\r\n' +
			'\r\n' +
			bodyText;
			
		client.write(
			response,
			function(){
				//alert('finished write');
			}
		);
	});
});