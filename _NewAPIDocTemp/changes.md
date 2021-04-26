Changes:

- update duktape library to version 2.6.0
- run all script instance startup routines on the same thread
- use javascript "strict mode"
- add default execution timeout of 500ms for instance startups/callback invocations
- replace socket overlapped IO with worker threads (fix Windows XP incompatibility)
- add global constants for addresses of exception vectors and memory mapped registers
- remove API.js, make all API functions native
- add better error throwing for all API functions
- rewrite API documentation file
- improve console.log's performance
- add remaining registers to `cop0` interface
- add events.onpifread(callback)
- add console.listen(inputListener)
- add script.keepalive(keepAlive)
- add script.timeout(milliseconds)
- add debug.skip()
- add debug.showmemory(address)
- add debug.showcommands(address)
- add debug.resume()
- add asm.encode(command)
- add asm.decode(opcode)
- add mem.setblock(address, data[, length))
- add mem.setstring(address, data[, length))
- add mem.ramSize
- add mem.romSize
- add pj64.open(romPath)
- add pj64.close()
- add exec(command)
- add require(id)

Changes that will break compatibility with old scripts:

- change all `events.on*` callbacks to receive more informative event objects
- rename `fs.writeFile` to `fs.writefile`
- rename `fs.readFile` to `fs.readfile`
- rename `mem.float`, `mem.double` to `mem.f32`, `mem.f64`
- rename global `float`, `double` type IDs to `f32`, `f64`
- move `gpr`, `ugpr`, `fpr`, `dfpr`, `cop0` interfaces into new `cpu` interface
- move `gpr.pc` to `cpu.pc`
- move `gpr.hi`/`ugpr.hi` to `cpu.hi`/`cpu.uhi`
- move `gpr.lo`/`ugpr.lo` to `cpu.hi`/`cpu.ulo`
- remove maskless version of `events.onopcode`
- remove redundant `rom` interface because `mem` interface can access ROM
- remove `GPR_ANY_ARG`, `GPR_ANY_TEMP`, `GPR_ANY_SAVE`

I am not sure what the situation is with `g_Reg->FAKE_CAUSE_REGISTER` and `g_Reg->CAUSE_REGISTER`,
but I changed `cpu.cop0.cause` to include them both.

- reads from `cpu.cop0.cause` now return `(g_Reg->FAKE_CAUSE_REGISTER | g_Reg->CAUSE_REGISTER)`
- writes to `cpu.cop0.cause` set both `g_Reg->FAKE_CAUSE_REGISTER` and `g_Reg->CAUSE_REGISTER`

---------------------
TODO:

- debug.stepping
- debug.breakhere no window option
- script auto-run feature?
- finish implementing new server/socket
- finish implementing new screen interface
- full test suite

- pj64.savestate(path)
- pj64.loadstate(path)
- pj64.reset(softReset = true)
- pj64.limitfps(limitFps = true)
- pj64.pause()
- pj64.resume()
- pj64.addmenu(null, caption, callback, menuKey)
- pj64.delmenu()

- DrawingContext:
   ctx.strokerect(x, y, width, height)
   ctx.moveto(x, y)
   ctx.lineto(x, y)
   ctx.stroke()
   ctx.print -> ctx.drawtext
   ctx.measuretext
   subtract status bar height from ctx.height


- formatting/cleanup printfs, comments etc

-----------------------------

var toolsMenu = pj64.addmenu(null, 'tools_menu', "Tools",);

pj64.addmenu(toolsMenu, "Kill Mario", function(){
    mem.u16[0x8033B21E] = 0;
});

---------------------------

backburner:
- write input history to a file
- debug.getsymbol?
- debug.setsymbol?
- pj64.savesetting(section, name, value)
- pj64.loadsetting(section, name)