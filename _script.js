// (A0 ptr parent_obj, A1 u8 gfx_id, A2 segptr behavior)
var proc_spawn_child_obj = 0x8029EDCC

var behavior_explosion = 0x13003510

var behavior_chuckya = 0x13000528
var gfx_chuckya = 0xDF

events.onexec(proc_spawn_child_obj, function(){
	if (gpr.a2 == behavior_explosion) {
		gpr.a1 = gfx_chuckya
		gpr.a2 = behavior_chuckya
	}
});

////////////

var mario_power = 0x8033B21E
var proc_goomba$collide = 0x802FFAF4

var goomba_collisions = 0

function killMario() {
	mem.u8[mario_power] = 0
}

events.onexec(proc_goomba$collide, function(){
	goomba_collisions++
	if(goomba_collisions >= 3){
		alert("DONT TOUCH MY GOOMBAS\n\t\t-Bowser")
		killMario()
	}
});
























