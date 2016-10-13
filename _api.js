Number.prototype.hex = function(len)
{
	var str = (this >>> 0).toString(16)
	while(str.length < len){
		str = "0" + str
	}
	return str
}

const u8 = 'u8', u16 = 'u16', u32 = 'u32',
      s8 = 's8', s16 = 's16', s32 = 's32',
	  float = 'float',  double = 'double'

const _typeSizes = {
	u8: 1, u16: 2, u32: 4,
	s8: 1, s16: 2, s32: 4,
	float: 4, double: 8
}

const _regNums = {
	r0:  0, at:  1, v0:  2, v1:  3,
	a0:  4, a1:  5, a2:  6, a3:  7,
	t0:  8, t1:  9, t2: 10, t3: 11,
	t4: 12, t5: 13, t6: 14, t7: 15,
	s0: 16, s1: 17, s2: 18, s3: 19,
	s4: 20, s5: 21, s6: 22, s7: 23,
	t8: 24, t9: 25, k0: 26, k1: 27,
	gp: 28, sp: 29, fp: 30, ra: 31
}

const system = {
	pause: function(){},
	resume: function(){}
}

const events = (function()
{
	var callbacks = {};
	var nextCallbackId = 0;
	return {
		on: function(hook, callback, tag)
		{
			this._stashCallback(callback);
			return _native.addCallback(hook, callback, tag);
		},
		onexec: function(addr, callback)
		{
			events.on('exec', callback, addr)
		},
		onread: function(addr, callback)
		{
			events.on('read', callback, addr)
		},
		onwrite: function(addr, callback)
		{
			events.on('write', callback, addr)
		},
		off: function(){},
		clear: function(){},
		_stashCallback: function(callback)
		{
			callbacks[nextCallbackId] = callback;
			return nextCallbackId++;
		},
		_unstashCallback: function()
		{
			
		},
	}
})();

const gpr = new Proxy({}, // todo dgpr for 64 bit
{
	get: function(obj, prop)
	{
		if (prop in _regNums)
		{
			return _native.getGPRVal(_regNums[prop])
		}
	},
	set: function(obj, prop, val)
	{
		if (prop in _regNums)
		{
			_native.setGPRVal(_regNums[prop], val)
		}
	}
})

const mem = {
	u8: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMInt(prop, 8, false);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMInt(prop, 8, val);
		}
	}),
	u16: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMInt(prop, 16, false);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMInt(prop, 16, val);
		}
	}),
	u32: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMInt(prop, 32, false);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMInt(prop, 32, val);
		}
	}),
	s8: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMInt(prop, 8, true);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMInt(prop, 8, val);
		}
	}),
	s16: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMInt(prop, 16, true);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMInt(prop, 16, val);
		}
	}),
	s32: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMInt(prop, 32, true);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMInt(prop, 32, val);
		}
	}),
	'float': new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMFloat(prop);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMFloat(prop, val);
		}
	}),
	'double': new Proxy({},
	{
		get: function(obj, prop)
		{
			return _native.getRDRAMFloat(prop, true);
		},
		set: function(obj, prop, val)
		{
			_native.setRDRAMFloat(prop, val, true);
		}
	}),
	bindvar: function(obj, baseAddr, name, type)
	{
		Object.defineProperty(obj, name,
		{
			get: function()
			{
				return mem[type][baseAddr]
			},
			set: function(val)
			{
				mem[type][baseAddr] = val
			}
		})
		return obj
	},
	bindvars: function(obj, list)
	{
		for(var i = 0; i < list.length; i++)
		{
			mem.bindvar(obj, list[i][0], list[i][1], list[i][2]);
		}
		return obj
	},
	bindstruct: function(obj, baseAddr, props)
	{
		for (var name in props)
		{
			var type = props[name]
			var size = _typeSizes[type]
			mem.bindvar(obj, baseAddr, name, type)
			baseAddr += size
		}
		return obj
	},
	typedef: function(props, proto)
	{
		var size = 0
		for (var name in props)
		{
			size += _typeSizes[props[name]]
		}
		var StructClass = function(baseAddr)
		{
			mem.bindstruct(this, baseAddr, props)
		}
		StructClass.sizeof = function()
		{
			return size
		}
		/*if(proto)
		{
			StructClass.prototype = proto
		}*/
		return StructClass
	}
}

function alert(text, caption){
	caption = caption || "";
	_native.msgBox(text, caption);
}


function Socket(fd)
{
	var _fd = fd;
	
	this.bufferSize = 2048;
	
	this.write = function(data, callback)
	{
		_native.write(_fd, data, callback)
	}

	var _ondata = function(data){};
	
	function _read(data)
	{
		_ondata(data);
		_native.read(_fd, this.bufferSize, _read)
	}
	
	this.on = function(eventType, callback)
	{
		switch(eventType)
		{
		case 'data':
			_ondata = callback;
			_native.read(_fd, this.bufferSize, _read)
			break;
		}
	}
}

function Server(settings)
{
	var _this = this;
	var _fd = _native.sockCreate()
	
	var _onconnection = function(socket){};
	
	_native.sockListen(_fd, settings.port || 80)
	
	// Intermediate callback
	//  convert clientFd to Socket and accept next client
	function _acceptClient(clientFd)
	{
		_onconnection(new Socket(clientFd))
		_native.sockAccept(_fd, _acceptClient)
	}
	
	this.listen = function(port)
	{
		_native.sockListen(_fd, port)
	}
	
	this.on = function(eventType, callback)
	{
		switch(eventType)
		{
		case 'connection':
			_onconnection = callback
			_native.sockAccept(_fd, _acceptClient)
			break;
		}
	}
}