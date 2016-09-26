/*

mem

 mem.u8[]
 mem.u16[]
 mem.u32[]
 mem.bindvar(obj, baseAddr, name, type)
 mem.bindvar(obj, vars)
 mem.bindstruct()
 mem.typedef()

events

 events.on(hook, callback, tag)
 events.onexec(address, callback)
 events.onread(address, callback)
 events.onwrite(address, callback)

gpr

 gpr.<register name>

alert(message)

_AddEvent(hook, callback, tag)
_SetGPRVal(regnum, val)
_GetGPRVal(regnum)
_GetRDRAMU8(address)
_GetRDRAMU16(address)
_GetRDRAMU32(address)
_SetRDRAMU8(address, value)
_SetRDRAMU16(address, value)
_SetRDRAMU32(address, value)

*/

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
	u8     : 1, u16     : 2, u32   : 4,
	s8     : 1, s16     : 2, s32   : 4,
	'float': 4, 'double': 8
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

const events = {
	on: function(hook, callback, tag)
	{
		return _AddCallback(hook, callback, tag)
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
	clear: function(){}
}

const gpr = new Proxy({}, // todo dgpr for 64 bit
{
	get: function(obj, prop)
	{
		if (prop in _regNums)
		{
			return _GetGPRVal(_regNums[prop])
		}
	},
	set: function(obj, prop, val)
	{
		if (prop in _regNums)
		{
			_SetGPRVal(_regNums[prop], val)
		}
	}
})

const mem = {
	u8: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _GetRDRAMU8(prop)
		},
		set: function(obj, prop, val)
		{
			_SetRDRAMU8(prop, val)
		}
	}),
	u16: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _GetRDRAMU16(prop)
		},
		set: function(obj, prop, val)
		{
			_SetRDRAMU16(prop, val)
		}
	}),
	u32: new Proxy({},
	{
		get: function(obj, prop)
		{
			return _GetRDRAMU32(prop)
		},
		set: function(obj, prop, val)
		{
			_SetRDRAMU32(prop, val)
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
	},
	bindvars: function(obj, list)
	{
		for(var i = 0; i < list.length; i++)
		{
			mem.bindvar(obj, list[i][0], list[i][1], list[i][2]);
		}
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
