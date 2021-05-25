Changes:

- update duktape library to version 2.6.0
- run all script instance startup routines on the same thread
- use javascript "strict mode"
- add default execution timeout of 500ms for instance startups/callback invocations
- replace socket overlapped IO with worker threads (fix Windows XP incompatibility)
- remove API.js, make all API functions native
- add better error throwing for all API functions
- improve console.log's performance
- reads from `cpu.cop0.cause` now return `(g_Reg->FAKE_CAUSE_REGISTER | g_Reg->CAUSE_REGISTER)`
- writes to `cpu.cop0.cause` set both `g_Reg->FAKE_CAUSE_REGISTER` and `g_Reg->CAUSE_REGISTER`
- add a ton of new functions

Changes that will break compatibility with old scripts:

- change parameters of `events.on*` callbacks to event objects
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

---------------------
TODO:

- fix open rom dialog appearing behind script render overlay
- finish implementing new server/socket
- native module loader
- stable pj64 open/close
- make sure ctx.fill() implementation is correct
- ctx.getimage(x, y, width, height)
- 2px black stroke by default
- bold by default
- events.onemustart(e: GenericEvent)
- events.onemustop(e: GenericEvent)

- ctx.pointer  Native pointer to the internal Direct2D render target. May be used by native modules for faster drawing.
- mem.pointer  Native pointer to Project64's N64 memory. May be used by native modules for faster memory access.

- formatting/cleanup printfs/todo comments/warnings etc
- full test suite

---------------------------

backburner:
- write input history to a file
- debug.getsymbol?
- debug.setsymbol?
- pj64.savesetting(section, name, value)
- pj64.loadsetting(section, name)
- pj64.addmenu(null, caption, callback, menuKey)
- pj64.delmenu()
- pj64.savestate(path)
- pj64.loadstate(path)
- pj64.reset(softReset = true)
- pj64.limitfps(limitFps = true)
- pj64.pause()
- pj64.resume()
- image.fixpalette() // remove duplicate values from the image.palette buffer and resize to fit. update ci data in image.data and update internal bitmap.
- autorun execution order
