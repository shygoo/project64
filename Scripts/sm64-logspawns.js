events.onexec(0x8029EDCC, function(){
	console.log("spawn_child(0x" + gpr.a0.hex() + ", " + gpr.a1 + ", 0x" + gpr.a2.hex() + ", " + gpr.a3 +")")
})