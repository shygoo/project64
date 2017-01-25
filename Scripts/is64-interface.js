// zelda debug is64

// todo instruction decoder interface

const is64_base = 0xB3FF0000;

const is64_r0 = is64_base + 0x00;
const is64_r1 = is64_base + 0x04;
const is64_r2 = is64_base + 0x14;
const is64_r3 = is64_base + 0x20;

var buffer = '';

events.onwrite(is64_r0, function(){
	console.log('WRITE R0' + gpr.pc.hex() + ' ' + gpr.t4.hex() + ' ' + gpr.a2.hex())
	debug.breakhere();
})

events.onwrite(is64_r1, function(){
	//console.log('WRITE R1 ' + gpr.pc.hex() + ' ' + gpr.t4.hex() + ' ' + gpr.a2.hex())
})

events.onwrite(is64_r2, function(){
	//console.log('WRITE R2 ' + gpr.pc.hex() + ' ' + gpr.t4.hex() + ' ' + gpr.a2.hex())
})

events.onwrite(is64_r3, function(){
	alert('wrote to R3');
	console.log('WRITE R3');
})

//events.onread(is64_r0, function(){
//	console.log('READ R0 ' + gpr.pc.hex())
//})

events.onexec(0x80006684, function(){
	if(gpr.t4 == is64_r0)
	{
		console.log('sending back is64')
		//gpr.t5 = 0x49533634; // send 'IS64' back to the game
	}
})

// 80006680

// 80002200 checks if device replied with IS64
// 8000221C