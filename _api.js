
// setup javascript api

var system = {
	pause: function(){},
	resume: function(){}
};

var events = {
	on: function on(hook, callback, tag){
		return _AddEvent(hook, callback, tag);
	},
	onexec: function(addr, callback){
		events.on('exec', callback, addr);
	},
	off: function(){},
	clear: function(){}
};

var _regNums = {
	r0:   0, at :  1, v0 :  2, v1 :  3, a0 :  4, a1 :  5, a2 :  6, a3 :  7,
	t0 :  8, t1 :  9, t2 : 10, t3 : 11, t4 : 12, t5 : 13, t6 : 14, t7 : 15,
	s0 : 16, s1 : 17, s2 : 18, s3 : 19, s4 : 20, s5 : 21, s6 : 22, s7 : 23,
	t8 : 24, t9 : 25, k0 : 26, k1 : 27, gp : 28, sp : 29, fp : 30, ra : 31
};

var gpr = new Proxy({},
{
	get: function(obj, prop)
	{
		if (prop in _regNums)
		{
			return _GetGPRVal(_regNums[prop]);
		}
	},
	set : function(obj, prop, val)
	{
		if (prop in _regNums)
		{
			_SetGPRVal(_regNums[prop], val);
		}
	}
});

// todo dgpr for 64 bit

var mem = {
	u8: new Proxy({}, {
		get: function(obj, prop
		{
			return _GetRDRAMU8(prop);
		},
		set: function(obj, prop, val)
		{
			_SetRDRAMU8(prop, val);
		}
	}),
	u16: function(){},
	u32: function(){},
};

alert("setup ok");