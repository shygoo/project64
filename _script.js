var mario = mem.bindvars({}, [
	[0x8033B1AC, 'x', float],
	[0x8033B1B0, 'y', float],
	[0x8033B1B4, 'z', float],
])

var server = new Server({port: 80})

server.on('connection', function(socket)
{
	socket.on('data', function(data)
	{
		if(data.byteLength == 12)
		{
			var floats = new Float32Array(data)
			
			alert('got data ' + floats[0] + ' ' + floats[1] + ' ' + floats[2]);
			
			mario.x = floats[0];
			mario.y = floats[1];
			mario.z = floats[2];
		}
	})
})

/*
var minServer = new Server({port: 81});

minServer.on('connection', function(socket)
{
	socket.on('data', function(data)
	{
		var responseBody = "hello world";
	
		var response =
			'HTTP/1.1 200 OK\r\n'+
			'Content-Type: text/html\r\n'+
			'Access-Control-Allow-Origin: *\r\n' +
			'Content-Length: 5\r\n' +
			'\r\n' +
			'hello'
	
		socket.write(response);
		socket.close();
	})
	
	socket.on('close', function(){
		//alert('connection closed');
	})
	
});



var server = new Server({port: 80});

var startpoint = 0x801A45B0
var limit = 883

//8033C5E8 alt
// 80381B4C write instr
mem.bindvar(this, 0x8033C600, 'lpPlayerCollisionTriangle', u32);

// 801AEB40

// 2300

server.on('connection', function(socket) {
	var AWAITING_HEADER = 0;
	var AWAITING_BODY = 1;
	
	var state = AWAITING_HEADER;
	
	socket.on('close', function() {
		alert('connection closed');
	});
	
	socket.on('data', function(data) {
		// var lines = data.toString().split(/\n/);
		// var firstLine = lines[0].split(" ");
		// var method = firstLine[0];
		// var path = firstLine[1];
		// var version = firstLine[2];
		// var querydata = path.split('?', 2);
		// if(querydata[1])
		// {
		// 	querydata = querydata[1].split('&');
		// }
		// var query = {};
		// if(querydata.length)
		// {
		// 	for(var i in querydata)
		// 	{
		// 		var spl = querydata[i].split('=');
		// 		query[spl[0]] = spl[1];
		// 	}
		// }
		// 
		// var headers = {};
		// 
		// for(var i = 1; lines[i].trim() != ''; i++)
		// {
		// 	var header = lines[i].split(': ', 2);
		// 	headers[header[0]] = header[1].trim();
		// }
		
		//var body = {
		//	method: method,
		//	path: path,
		//	query: query,
		//	headers: headers
		//};
        //
		//var bodyText = JSON.stringify(body);
		
		// todo some utility functions for this buffer stuff
		
		var collisionData = mem.getblock(startpoint, 0x30 * limit);
		
		var output = new ArrayBuffer(4 + collisionData.byteLength);
		
		// first four bytes = player idx, rest = collision data
		
		var dv = new DataView(output);
		dv.setUint32(0, lpPlayerCollisionTriangle - startpoint); // offset of player triangle
		
		var u8arr = new Uint8Array(output)
		u8arr.set(new Uint8Array(collisionData), 4);
		
		var response =
			'HTTP/1.1 200 OK\r\n'+
			'Content-type: application/octet-stream\r\n'+
			'Access-Control-Allow-Origin: *\r\n' +
			'Content-length: ' + u8arr.length + '\r\n' +
			'\r\n' +
			new Buffer(u8arr);
			
		socket.write(response, function() {
			//alert('finished write');
		});
		
	});
});
*/