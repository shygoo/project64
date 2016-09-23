/*
Duktape.modSearch = function(id)
{
	return "exports.test = function(){alert(1)}";
}
var mod = require('phony.js');
mod.test();
*/

var SM64Object = mem.typedef({
	_00_u8: u8,
	_01_u8: u8,
	_02_u8: u8,
	_03_u8: u8,
	_04_u32: u32
});

var objects = [];

for(var i = 0; i < 240; i++)
{
	objects.push(new SM64Object(0x8033D488 + i * 0x260));
}

// proc_levelscript_00
events.onexec(0x8037E2C4, function() {
	alert(objects[0]._04_u32.hex(8));
})