
alert('Script system okay');

proc_beh_mario_a = 0x802CB1C0; // mario behavior function

onexec(proc_beh_mario_a, function(){
	alert('proc_beh_mario_a was called');
});
