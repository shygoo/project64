
// http api draft

function HTTPServer(settings)
{
	// settings.port/defaultHeaders

	var server = new Server({port: 80});
	
	var _onrequest = function(){};
	
	// httprequest.on/data on/end
	
	server.on('connection', function(socket)
	{
		var haveCompleteHeaders = false;
		var haveCompleteData = false;
		
		var headers = {};
		
		var headerData = '';
		var bodyData = '';
		
		var httpRequest = {
			headers: headers,
			data: '',
			_ondata_base: function(data){
				
			},
			_ondata: function(data){},
			_onend: function(){},
		};
		
		var contentLength = 0;
		
		socket.on('data', function(data)
		{
			data = data.toString();
			if(!haveCompleteHeaders)
			{
				var components = data.match(/(.+)(\r*\n\r*\n)?(.+)?/)
				// 0 = fulltext, 1 = headerData, 2 = br, 3 = bodyData 
				if(components[1])
				{
					headerData += components[1];
					
					if(components[2]) // double line break
					{
						haveCompleteHeaders = true;
						httpRequest.headers = HTTPServer.parseHeaders(headerData);
						
						contentLength = httpRequest.headers.getValue('Content-length') | 0;	
						
						if(components[3])
						{
							// if data comes 'early', buffer it
							httpRequest.data = components[3]
							if(httpRequest.data.length == contentLength)
							{
								// already have all the data
								_onrequest(httpRequest, null)
							}
						}
					}
				}
			}
			else if(!haveCompleteData)
			{
				httpRequest.data += data;
				
			}
		})
		
		this.on = function(evt, callback)
		{
			switch(evt)
			{
			case 'request':
				_onrequest = callback;
				break;
			}
		}
		
		socket.on('close', function()
		{
			
		})
	})
}

const http = {};

http.Headers = function(headerData)
{
	var rawHeaderLines = headerData.split(/\r*\n/);
	var rawStatusLine = rawHeaderLines.shift();
	
	var statusComponents = rawStatusLine.match(/(.+?) (.+?) (.+?)\/(.+)/);
	
	if(statusComponents == null)
	{
		return;
	}
	
	statusComponents.shift();
	this.method = statusComponents[0];
	this.path = statusComponents[1];
	this.protocol = statusComponents[2];
	this.version = statusComponents[3];
	
	this.attributes = {}
	
	for(var i in rawHeaderLines)
	{
		var headerComponents = rawHeaderLines[i].match(/(.+?): *(.+)/);
		
		if(headerComponents == null)
		{
			return;
		}
		
		headerComponents.shift();
		
		var name = headerComponents[0];
		var value = headerComponents[1];
		
		name = name.toLowerCase();
		
		if(name)
		{
			this.attributes[name] = value;
		}
	}
}

http.Headers.prototype.getValue = function(name)
{
	name = name.toLowerCase()
	
	if(name in this.attributes)
	{
		return this.attributes[name];
	}
}

http.Headers.prototype.raw = function()
{
	var rawHeaders = this.protocol + '/' +
	                 this.version + ' ' +
					 this.status + ' ' +
					 this.statusRemark + '\r\n';
	
	for(var i in this.attributes)
	{
		rawHeaders += i + ': ' + this.attributes[i] + '\r\n';
	}
	
	return rawHeaders;
}


/*

http.createServer(function(request, response)
{
	response.setHeader()
	response.end(data)
})

*/
