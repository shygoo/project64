var game = mem.bindvars({}, [
	[0x8033B1AC, 'mario_x', u32],
	[0x8033B1B0, 'mario_y', u32],
	[0x8033B1B4, 'mario_z', u32]
]);

function processData(data)
{
	var floatTriplet = new Uint32Array(data)
	game.mario_x = floatTriplet[0]
	game.mario_y = floatTriplet[1]
	game.mario_z = floatTriplet[2]
}

var keepConnected = true;

var serverThread = new Thread(function()
{
	var server = _CreateServer(8080)
	var client = _SockAccept(server)
	
	if(client == -1)
	{
		alert("Something broke")
		return;
	}
	
	alert("Client connected!")
	
	while(keepConnected)
	{
		// Wait for 12 bytes then pass to processData
		var data = _ReceiveBytes(client, 12)
		processData(new ArrayBuffer(data))
	}
});

serverThread.start()
