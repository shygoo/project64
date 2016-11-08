
var spawnhack = {
	enabled: false,
	object: 0x00
}

events.onexec(0x800FC9C0, function()
{
	if(spawnhack.enabled)
	{
		mem.u8[gpr.a1 + 2] = spawnhack.object;
		console.log("force-spawned " + spawnhack.object.hex(2))
	}
})

/*

00 (N-Z)
01 rolling rock
02 sandwich/cake etc
03 meat with bone
04 (Gordo)
05 cannon
06 blue spinning prism with eye
07 bouncing spring
08 (Glunk) green/yellow gel with red flower petals
09 ?? explodes
0A weird thing with blue hat
0B purple drop with propellers
0C black circle with hidden frog
0D green circle with hidden yellow thing that shoots bubbles
0E (Poppy Bro. Jr.)
0F (Bivolt)
10 rolling log
11 big plant that eats you
12 red crab
13 (Bivolt) (again?)
14 (Sir Kibble)
15 purple thing with skull mask, throws bones
16 4-legged black spider
17 (large I3)
18 yellow underground thing with large pincers
19 ?? explodes (again?)
1A (Bonehead)
1B ghost-like with red ball in head
1C running fire
1D (Bo)
1E black ball with turtle shell
1F mole thing
20 ghostly diglet-like thing
21 red flopping fish
22 green bird that throws wreath hat
23 blue squid-like thing with one eye, bites
24 water drop that extends upward, contains red ball
25 (Pedo)
26 ghost-like thing that throws purple fire
27 smiling ball with red hat
28 stack of three rocks
29 ?? invisible
2A black orb with eyes, acts like a volcano http://shygoo.net/pic/lpwyc.png
2B (Hack)
2C yellow bird that drops bombs http://shygoo.net/pic/kxpry.png
2D fish-like arrangement of triangles, projects head http://shygoo.net/pic/ferct.png
2E snowy bird http://shygoo.net/pic/hcvns.png
2F sawblade http://shygoo.net/pic/hnvlg.png
30 purple orb with 4 gray appendages, electric http://shygoo.net/pic/rwmtn.png

...

*/