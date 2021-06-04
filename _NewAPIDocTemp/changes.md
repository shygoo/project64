Changes:

- update duktape library to version 2.6.0
- add autorun config window to run scripts on PJ64 startup
- add `Autorun...` to script list context menu
- rewrite ScriptSystem and ScriptInstance
  - remove API.js, make all API functions native
  - run all script instance startup routines on the same thread
  - add default execution timeout of 500ms for instance startups/callback invocations
  - replace socket overlapped IO with worker threads (fix Windows XP incompatibility)
  - print a proper error stack to the console when there is a JS error
  - use JS "strict mode"

Notable API additions:

- implement basic JS `require()` with native module support
- add `events` hooks: `onpifread`, `onsptask`, `onpidma`, `onmousedown`, `onmouseup`, `onmousemove`
- add `debug` functions: `resume`, `step`, `skip`, `showmemory`, `showcommands`
- add optional `silent` parameter to `debug.breakhere` to prevent commands window from showing
- add `exec()` function for shell command execution
- add remaining cop0 registers and fcr31
- add `N64Image` class for texture dumping/manipulation
- add `DrawingContext` class for screen drawing

Changes that will break compatibility with old scripts:

- change parameter(s) of `events.on*` callbacks to event objects
- rename `fs.writeFile` to `fs.writefile`
- rename `fs.readFile` to `fs.readfile`
- rename `mem.float`, `mem.double` to `mem.f32`, `mem.f64`
- rename global `float`, `double` type IDs to `f32`, `f64`
- move `gpr`, `ugpr`, `fpr`, `dfpr`, `cop0` interfaces into new `cpu` interface
- move `gpr.pc` to `cpu.pc`
- move `gpr.hi`/`ugpr.hi` to `cpu.hi`/`cpu.uhi`
- move `gpr.lo`/`ugpr.lo` to `cpu.hi`/`cpu.ulo`
- remove maskless variant of `events.onopcode`
- remove `screen` interface (replaced by `DrawingContext`)
- remove redundant `rom` interface because `mem` interface can access ROM
- remove `GPR_ANY_ARG`, `GPR_ANY_TEMP`, `GPR_ANY_SAVE`

I did something lazy to get plugin-independent screen drawing to work without flicker problems.
After the GFX plugin draws to the main window, the pixels are copied to a borderless overlay window that hovers over the main window's client area.
Then the user script can draw to the overlay window (instead of drawing to the main window and fighting with the GFX plugin like it was before).
I don't know of a better way to do this that wouldn't require changes to the plugin spec.

I'm aware of a few issues with my approach:

- `BitBlt` is used to obtain a bitmap of the main window's game screen (before being drawn to the overlay window via Direct2D). On Windows 7 this seems to always yield a black box unless a Direct3D plugin is used (i.e. only Jabo's is compatible).
- Modal windows appear behind the overlay window (e.g. the Open ROM dialogue). There is probably a simple fix for this but I haven't figured it out yet.
- The overlay window probably lags by 1 frame which is not great for speedrunning.
- The main window is unfocused when the overlay window is clicked. I fixed this by automatically refocusing the main window, but it makes the title bar flicker sometimes.
- The drawing functions rely on Direct2D/DirectWrite so they won't work in Windows XP. I don't think this matters but I thought I should make note of it.

The overlay window is only active/visible while the debugger is enabled and at least one `events.ondraw` callback is registered, so these issues won't affect normal gameplay.

---------------------
TODO:

- fix open rom dialog appearing behind script render overlay
- finish implementing new server/socket
- stable pj64 open/close
- make sure ctx.fill() implementation is correct
- ctx.getimage(x, y, width, height)
- events.onemustart(e: GenericEvent)
- events.onemustop(e: GenericEvent)
- ctx.pointer  Native pointer to the internal Direct2D render target. May be used by native modules for faster drawing.
- mem.pointer  Native pointer to Project64's N64 memory. May be used by native modules for faster memory access.
- formatting/cleanup printfs/todo comments/warnings etc
- full test suite
- expose synchronization object to native modules

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
