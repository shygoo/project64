var handlers = {
    '.end': cmd_end,
    '.eval': cmd_eval,
    '.clear': cmd_clear,
    'all': test_all,
    'mem': test_mem,
    'events': test_events,
    'number_hex': test_number_hex,
    'extra_timeout': test_extra_timeout,
    'extra_events': test_extra_events
};

greeting();

script.listen(function(input) {
    input = input.split(' ');
    var command = input.shift();
    var args = input;

    if(command in handlers)
    {
        handlers[command](args);
    }
    else
    {
        console.log("unknown command '" + command + "'");
    }
});

function greeting()
{
    console.clear();
    console.log("PJ64 JS API validation");
    console.log("----------------------------------");
    console.log("Commands: ");
    console.log("----------------------------------");
    for(var key in handlers)
    {
        console.log(" - " + key);
    }
    console.log("----------------------------------");
}

function runtest(caption, func)
{
    caption += ' ';
    while(caption.length < 32) {
        caption += '.';
    }

    console.print(caption + ' ');

    try {
        var res = func();
        console.log(res ? '[OK]' : '[FAIL]');
        return res;
    } catch(e) {
        console.log('[ERROR]');
        console.log('--------------------------------');
        console.log(e.stack);
        console.log('--------------------------------');
        return false;
    }
}

/**************************************/

function cmd_clear()
{
    greeting();
}

function cmd_eval(args)
{
    var code = args.join(' ');
    console.log('[<]: ' + code);

    var result;

    try {
        result = eval(code)
    } catch(e) {
        result = e.stack;
    }

    console.log('[>]: ' + result + '\n');
}

function cmd_end()
{
    script.listen(undefined);
}

/**************************************/

function test_extra_events()
{
    console.log("Extra events test started");
    console.log("Waiting for interrupt @ 0x80000180...")
    //var cbid_test_program_completed;
    
    var cbid_interrupt_trapped = events.onexec(0x80000180, function() {
        events.remove(cbid_interrupt_trapped);
        console.log("Interrupt trapped. Injecting events test program...");
        // ...
        console.log("Redirecting PC...");
        gpr.pc = 0x80400000;

        var cbid_test_program_completed = events.onexec(0x80400020, function() {
            events.remove(cbid_test_program_completed);
            console.log("Resuming game...");
            gpr.pc = 0x80000180;
        });
    });
}

function test_events()
{
    function fn(){}
    var n = 0xFFFFFFFF;

    runtest('events: onexec/remove', function() {
        var id = events.onexec(n, fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });

    runtest('events: onread/remove', function() {
        id = events.onread(n, fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });

    runtest('events: onwrite/remove', function() {
        id = events.onwrite(n, fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });

    runtest('events: onopcode/remove', function() {
        id = events.onopcode(n, n, n, fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });

    runtest('events: opopcode2/remove', function() {
        id = events.onopcode(n, n, fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });

    runtest('events: ongprvalue/remove', function() {
        id = events.ongprvalue(n, n, n, fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });

    runtest('events: ondraw/remove', function() {
        id = events.ondraw(fn);
        try { events.remove(id); } catch(e) { return false; }
        return true;
    });
}

function test_number_hex()
{
    runtest('number.hex', function(){
        return (
            (-1).hex() == 'FFFFFFFF' &&
            (0x64).hex() == '00000064' &&
            (0x64).hex(16) == '0000000000000064' &&
            (0x64).hex(2) == '64'
        );
    });
}

function test_all()
{
    test_mem();
    test_events();
    test_asm();
    test_number_hex();

    var id = events.onexec(0x80000180, function(){
        console.log("trapped 0x80000180");
        //events.remove(id);
    });
}

function test_mem()
{
    // 803FFF00
    const TEST_AREA_SIZE = 256;
    const TEST_AREA_ADDR = 0x80400000 - TEST_AREA_SIZE;
    
    const o_f64 = 0x00;
    const o_f32 = 0x08;
    const o_u32 = 0x0C;
    const o_s32 = 0x10;
    const o_u16 = 0x14;
    const o_s16 = 0x16;
    const o_u8  = 0x18;
    const o_s8  = 0x19;

    const p_f64 = TEST_AREA_ADDR + o_f64;
    const p_f32 = TEST_AREA_ADDR + o_f32;
    const p_u32 = TEST_AREA_ADDR + o_u32;
    const p_s32 = TEST_AREA_ADDR + o_s32;
    const p_u16 = TEST_AREA_ADDR + o_u16;
    const p_s16 = TEST_AREA_ADDR + o_s16;
    const p_u8  = TEST_AREA_ADDR + o_u8;
    const p_s8  = TEST_AREA_ADDR + o_s8;

    const structProps = {
        'v_f64': f64,
        'v_f32': f32,
        'v_u32': u32,
        'v_s32': s32,
        'v_u16': u16,
        'v_s16': s16,
        'v_u8':  u8,
        'v_s8':  s8
    };

    const arrProps = [
        [ p_f64, 'v_f64', f64 ],
        [ p_f32, 'v_f32', f32 ],
        [ p_u32, 'v_u32', u32 ],
        [ p_s32, 'v_s32', s32 ],
        [ p_u16, 'v_u16', u16 ],
        [ p_s16, 'v_s16', s16 ],
        [ p_u8,  'v_u8',  u8  ],
        [ p_s8,  'v_s8',  s8  ]
    ];

    function resetBuffer()
    {
        for(var i = 0; i < TEST_AREA_SIZE; i++)
        {
            mem.u8[TEST_AREA_ADDR + i] = 0x64;
            if(mem.u8[TEST_AREA_ADDR + i] != 0x64)
            {
                throw new Error("failed to clear memory");
            }
        }
    }

    function rawSetValues()
    {
        resetBuffer();
        mem.f64[p_f64] = 1;
        mem.f32[p_f32] = 2;
        mem.u32[p_u32] = 12345678;
        mem.s32[p_s32] = -12345678;
        mem.u16[p_u16] = 12345;
        mem.s16[p_s16] = -12345;
        mem.u8[p_u8] = 123;
        mem.s8[p_s8] = -123;
    }

    function objSetValues(obj)
    {
        resetBuffer();
        obj.v_f64 = 1;
        obj.v_f32 = 2;
        obj.v_u32 = 12345678;
        obj.v_s32 = -12345678;
        obj.v_u16 = 12345;
        obj.v_s16 = -12345;
        obj.v_u8 = 123;
        obj.v_s8 = -123;
    }

    function checkValues()
    {
        return (
            mem.f64[p_f64] == 1 &&
            mem.f32[p_f32] == 2 &&
            mem.u32[p_u32] == 12345678 &&
            mem.s32[p_s32] == -12345678 &&
            mem.u16[p_u16] == 12345 &&
            mem.s16[p_s16] == -12345 &&
            mem.u8[p_u8] == 123 &&
            mem.s8[p_s8] == -123
        );
    }

    var basicMemRes = runtest('mem: basic functionality', function(){
        try { var t = mem.u8[0x80000000]; }
        catch(e){ return false; }
        return true;
    });

    if(!basicMemRes)
    {
        console.log("mem test aborted -- make sure a game is running.");
        return;
    }


    runtest('mem: array read/write', function() {
        rawSetValues();
        return checkValues();
    });

    runtest('mem: bindvar', function() {
        var obj = {};
        arrProps.forEach(function(prop) {
            var addr = prop[0];
            var name = prop[1];
            var type = prop[2];
            mem.bindvar(obj, addr, name, type);
        })
        objSetValues(obj);
        return checkValues();
    });

    runtest('mem: bindvars', function() {
        var obj = {};
        mem.bindvars(obj, arrProps);
        objSetValues(obj);
        return checkValues();
    });

    runtest('mem: bindstruct', function() {
        var obj = {};
        mem.bindstruct(obj, TEST_AREA_ADDR, structProps);
        objSetValues(obj);
        return checkValues();
    });

    runtest('mem: typedef', function() {
        var TestType = mem.typedef(structProps);
        var obj = new TestType(TEST_AREA_ADDR);
        objSetValues(obj);
        return checkValues();
    });
}

function test_extra_timeout()
{
    runtest('script: timeout', function() {
        console.log('[????]');
        console.log('[Expect RangeError in 1000ms...]')
        script.timeout(1000);
        for(;;);
    });
}

function test_asm()
{
    const gprNames = [
        'r0', 'at', 'v0', 'v1', 'a0', 'a1', 'a2', 'a3',
        't0', 't1', 't2', 't3', 't4', 't5', 't6', 't7',
        's0', 's1', 's2', 's3', 's4', 's5', 's6', 's7',
        't8', 't9', 'k0', 'k1', 'gp', 'sp', 'fp', 'ra'
    ];

    runtest('asm: gprname', function() {
        for(var i = 0; i < 32; i++)
        {
            if(gprNames[i] != asm.gprname(i))
            {
                return false;
            }
        }
        return true;
    });
}