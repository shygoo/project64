
alert('Script system okay');

proc_beh_mario_a = 0x802CB1C0; // mario behavior function

events.onexec(proc_beh_mario_a, function(){
	alert('proc_beh_mario_a was called\n' +
		'a0: ' + gpr.a0.toString(16) + '\n' +
		'a1: ' + gpr.a1.toString(16) + '\n' +
		'a2: ' + gpr.a2.toString(16) + '\n' +
		'a3: ' + gpr.a3.toString(16) + '\n'
	);
});
