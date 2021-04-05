Changes:

- update duktape library to version 2.6.0
- run all script instance startup routines on the same thread
- use javascript "strict mode"
- add default execution timeout of 500ms for instance startups/callback invocations
- replace socket overlapped IO with worker threads (fix Windows XP incompatibility)
- remove API.js, make all API functions native
- add better error throwing for all API functions
- rewrite API documentation file
- add remaining registers to `cop0` interface
- add script.listen(inputListener) for console input handling
- add script.keepalive(bKeepAlive) to keep instance from automatically being destroyed
- add script.timeout(ms) for timeout override
- add debug.skip() for CPU command skipping
- add debug.showmemory(address) to show the memory window
- add debug.showcommands(address) to show the commands window
- add asm.encode(asm)
- add asm.decode(opcode)
- add mem.ramsize
- add mem.romsize

Changes that will break compatibility with old scripts:

- move `gpr`, `ugpr`, `fpr`, `dfpr`, `cop0` interfaces into new `cpu` interface
- move `gpr.pc` to `cpu.pc`
- move `gpr.hi`/`ugpr.hi` to `cpu.hi`/`cpu.uhi`
- move `gpr.lo`/`ugpr.lo` to `cpu.hi`/`cpu.ulo`
- remove redundant `rom` interface because `mem` interface can access ROM
- remove redundant `pc` argument from `events.onexec/onopcode/ongprvalue` callbacks since there is `gpr.pc`
- change the `address` argument of `events.onread/onwrite` callbacks to more informative `CPUReadWriteEvent` object
- remove `GPR_ANY_ARG`, `GPR_ANY_TEMP`, `GPR_ANY_SAVE`

I am not sure what the situation is with `g_Reg->FAKE_CAUSE_REGISTER` and `g_Reg->CAUSE_REGISTER`,
but I changed `cpu.cop0.cause` to include them both.

- reads from `cpu.cop0.cause` now return `(g_Reg->FAKE_CAUSE_REGISTER | g_Reg->CAUSE_REGISTER)`
- writes to `cpu.cop0.cause` set both `g_Reg->FAKE_CAUSE_REGISTER` and `g_Reg->CAUSE_REGISTER`

---------------------
TODO:

- script auto-run feature?
- finish implementing new server/socket
- finish implementing new events interface
- finish implementing new screen interface
- full test suite
- mem.setblock
- mem.saveaddress
- debug.getsymbol?
- debug.setsymbol?
- debug.resume()

- Duktape.modSearch/require()
- formatting/cleanup printfs, comments etc

- pj64.openrom()
- pj64.closerom()
- pj64.savestate(path)
- pj64.loadstate(path)
- pj64.reset(softReset = true)
- pj64.limitfps(limitFps = true)
- pj64.pause()
- pj64.resume()
- pj64.savesetting(section, name, value)
- pj64.loadsetting(section, name)


- pj64.addmenu(null, caption, callback, menuKey)
- pj64.delmenu()

-----------------------------

var toolsMenu = pj64.addmenu(null, 'tools_menu', "Tools",);

pj64.addmenu(toolsMenu, "Kill Mario", function(){
    mem.u16[0x8033B21E] = 0;
});



---------------------------

backburner:

buffer console.log()
write input history to a file

```
events.onwrite(PI_WR_LEN_REG, function(val) {
    var cartAddress = mem.u32[PI_CART_ADDR_REG];
    var dramAddress = mem.u32[PI_DRAM_ADDR_REG];
    //var length = e.value;
    console.log(cartAddress.hex(), dramAddress.hex(), val.hex());

    //console.log(JSON.stringify(e))
});
```