// nodejs buffer defined in duktape
declare interface Buffer {} 
declare interface MemTypeConstructor {}

declare class AddressRange {
    constructor(start: number, end: number);
    start: number;
    end: number;
}

declare class EmuEvent {
    callbackId: number;
}

declare class CPUEvent extends EmuEvent {
    pc: number;
}

declare class CPUExecEvent extends CPUEvent {}

class CPUReadWriteEvent extends CPUEvent {
    address: number;
    fpu: boolean;
    reg: number;
    type: number;
    value: number;
    valueHi: number;
}

declare class CPUOpcodeEvent extends CPUEvent {
    opcode: number;
}

declare class CPURegValueEvent extends CPUEvent {
    value: number;
    reg: number;
}

declare class ConsoleModule {
    log(message, ...optionalParams): void;
    print(message, ...optionalParams): void;
    clear(): void;
    listen(inputListener: (input: string) => void): void;
}

declare class RegBase {
    0: number;
    1: number;
    2: number;
    3: number;
    4: number;
    5: number;
    6: number;
    7: number;
    8: number;
    9: number;
    10: number;
    11: number;
    12: number;
    13: number;
    14: number;
    15: number;
    16: number;
    17: number;
    18: number;
    19: number;
    20: number;
    21: number;
    22: number;
    23: number;
    24: number;
    25: number;
    26: number;
    27: number;
    28: number;
    29: number;
    30: number;
    31: number
}

declare class GPRBase extends RegBase {
    r0: number;
    at: number;
    v0: number;
    v1: number;
    a0: number;
    a1: number;
    a2: number;
    a3: number;
    t0: number;
    t1: number;
    t2: number;
    t3: number;
    t4: number;
    t5: number;
    t6: number;
    t7: number;
    s0: number;
    s1: number;
    s2: number;
    s3: number;
    s4: number;
    s5: number;
    s6: number;
    s7: number;
    t8: number;
    t9: number;
    k0: number;
    k1: number;
    gp: number;
    sp: number;
    fp: number;
    ra: number;
    hi: number;
    lo: number;
}

declare class FPRModule extends RegBase {
    f0: number;
    f1: number;
    f2: number;
    f3: number;
    f4: number;
    f5: number;
    f6: number;
    f7: number;
    f8: number;
    f9: number;
    f10: number;
    f11: number;
    f12: number;
    f13: number;
    f14: number;
    f15: number;
    f15: number;
    f16: number;
    f17: number;
    f18: number;
    f19: number;
    f20: number;
    f21: number;
    f22: number;
    f23: number;
    f24: number;
    f25: number;
    f26: number;
    f27: number;
    f28: number;
    f29: number;
    f30: number;
    f31: number;
}

declare class DFPRModule extends RegBase {
    f0: number;
    f2: number;
    f4: number;
    f6: number;
    f8: number;
    f10: number;
    f12: number;
    f14: number;
    f16: number;
    f18: number;
    f20: number;
    f22: number;
    f24: number;
    f26: number;
    f28: number;
    f30: number;
}

declare class GPRModule extends GPRBase { pc: number; }
declare class UGPRModule extends GPRBase {}

declare class COP0Module {
    index: number;
    random: number;
    entrylo0: number;
    entrylo1: number;
    context: number;
    pagemask: number;
    wired: number;
    badvaddr: number;
    count: number;
    entryhi: number;
    compare: number;
    status: number;
    cause: number;
    epc: number;
    config: number;
    taglo: number;
    taghi: number;
    errorepc: number;
}

declare class EventsModule {
    onexec(address: number | AddressRange, callback: (e: CPUExecEvent) => void): number;
    onread(address: number | AddressRange, callback: (e: CPUReadWriteEvent) => void): number;
    onwrite(address: number | AddressRange, callback: (e: CPUReadWriteEvent) => void): number;
    onopcode(address: number | AddressRange, opcode: number, callback: (e: CPUOpcodeEvent) => void): number;
    onopcode(address: number | AddressRange, opcode: number, mask: number, callback: (e: CPUOpcodeEvent) => void): number;
    ongprvalue(address: number | AddressRange, registers: number, value: number, callback: (e: CPURegValueEvent) => void): number;
    onpifread(callback: () => void): number;
    ondraw(callback: () => void): number;
    remove(callbackId: number): boolean;
}

declare class DebugModule {
    breakhere(): void;
}

declare class ASMModule {
    gprname(registerIndex: number): void;
}

declare class FSStats {
    constructor(path: string);
    constructor(fd: number);
    dev: number;
    ino: number;
    mode: number;
    nlink: number;
    gid: number;
    rdev: number;
    size: number;
    atimeMs: number;
    mtimeMs: number;
    ctimeMs: number;
    atime: Date;
    mtime: Date;
    ctime: Date;
    isDirectory(): boolean;
    isFile(): boolean;
}

declare class FSModule {
    Stats: class=FSStats;
    open(path: string, mode: string): number;
    close(number): void;
    write(fd: number, buffer: string | Buffer | ArrayBuffer, offset?: number, length?: number, position?: number): number;
    writeFile(path: string, buffer: string | Buffer | ArrayBuffer): boolean;
    read(fd: number, buffer: Buffer | ArrayBuffer, offset: number, length: number, position: number) : number;
    readFile(path: string): Uint8Array;
    fstat(fd: number): FSStats;
    stat(path: string): FSStats;
    unlink(path: string): boolean;
    mkdir(path: string): boolean;
    rmdir(path: string): boolean;
    readdir(path: string): string[];
}

declare class MemModule {
    u32: any;
    u16: any;
    u8:  any;
    s32: any;
    s16: any;
    s8:  any;
    f64: any;
    f32: any;
    ramsize: number;
    romsize: number;
    getblock(address: number, length: number): Uint8Array;
    setblock(address: number, data: Buffer, length?: number): void;
    getstring(address: number, maxLength?: number): string;
    setstring(address: number, data: Buffer, length?: number): void;
    bindvar(object: Object, address: number, name: string, type: number): void;
    bindvars(object: Object, vars: any[]) : Object
    bindstruct(object: Object, address: number, properties: Object): Object;
    typedef(properties: Object): MemTypeConstructor;
}

declare class CPUModule {
    pc: number;
    hi: number;
    lo: number;
    gpr: GPRModule;
    ugpr: GPRModule;
    fpr: FPRModule;
    dfpr: DFPRModule;
    cop0: COP0Module;
}

declare const ADDR_ANY: AddressRange;
declare const ADDR_KUSEG: AddressRange;
declare const ADDR_KSEG0: AddressRange;
declare const ADDR_KSEG1: AddressRange;
declare const ADDR_KSEG2: AddressRange;
declare const ADDR_RDRAM: AddressRange;
declare const ADDR_RDRAM_UNC: AddressRange;
declare const ADDR_CART_ROM: AddressRange;
declare const ADDR_CART_ROM_UNC: AddressRange;

declare const u8: number;
declare const u16: number;
declare const u32: number;
declare const s8: number;
declare const s16: number;
declare const s32: number;
declare const f32: number;
declare const f64: number;

declare const GPR_R0: number;
declare const GPR_AT: number;
declare const GPR_V0: number;
declare const GPR_V1: number;
declare const GPR_A0: number;
declare const GPR_A1: number;
declare const GPR_A2: number;
declare const GPR_A3: number;
declare const GPR_T0: number;
declare const GPR_T1: number;
declare const GPR_T2: number;
declare const GPR_T3: number;
declare const GPR_T4: number;
declare const GPR_T5: number;
declare const GPR_T6: number;
declare const GPR_T7: number;
declare const GPR_S0: number;
declare const GPR_S1: number;
declare const GPR_S2: number;
declare const GPR_S3: number;
declare const GPR_S4: number;
declare const GPR_S5: number;
declare const GPR_S6: number;
declare const GPR_S7: number;
declare const GPR_T8: number;
declare const GPR_T9: number;
declare const GPR_K0: number;
declare const GPR_K1: number;
declare const GPR_GP: number;
declare const GPR_SP: number;
declare const GPR_FP: number;
declare const GPR_RA: number;

declare const RDRAM_CONFIG_REG       : 0xA3F00000;
declare const RDRAM_DEVICE_TYPE_REG  : 0xA3F00000;
declare const RDRAM_DEVICE_ID_REG    : 0xA3F00004;
declare const RDRAM_DELAY_REG        : 0xA3F00008;
declare const RDRAM_MODE_REG         : 0xA3F0000C;
declare const RDRAM_REF_INTERVAL_REG : 0xA3F00010;
declare const RDRAM_REF_ROW_REG      : 0xA3F00014;
declare const RDRAM_RAS_INTERVAL_REG : 0xA3F00018;
declare const RDRAM_MIN_INTERVAL_REG : 0xA3F0001C;
declare const RDRAM_ADDR_SELECT_REG  : 0xA3F00020;
declare const RDRAM_DEVICE_MANUF_REG : 0xA3F00024;

declare const SP_MEM_ADDR_REG        : 0xA4040000;
declare const SP_DRAM_ADDR_REG       : 0xA4040004;
declare const SP_RD_LEN_REG          : 0xA4040008;
declare const SP_WR_LEN_REG          : 0xA404000C;
declare const SP_STATUS_REG          : 0xA4040010;
declare const SP_DMA_FULL_REG        : 0xA4040014;
declare const SP_DMA_BUSY_REG        : 0xA4040018;
declare const SP_SEMAPHORE_REG       : 0xA404001C;

declare const SP_PC_REG              : 0xA4080000;
declare const SP_IBIST_REG           : 0xA4080004;

declare const DPC_START_REG          : 0xA4100000;
declare const DPC_END_REG            : 0xA4100004;
declare const DPC_CURRENT_REG        : 0xA4100008;
declare const DPC_STATUS_REG         : 0xA410000C;
declare const DPC_CLOCK_REG          : 0xA4100010;
declare const DPC_BUFBUSY_REG        : 0xA4100014;
declare const DPC_PIPEBUSY_REG       : 0xA4100018;
declare const DPC_TMEM_REG           : 0xA410001C;

declare const DPS_TBIST_REG          : 0xA4200000;
declare const DPS_TEST_MODE_REG      : 0xA4200004;
declare const DPS_BUFTEST_ADDR_REG   : 0xA4200008;
declare const DPS_BUFTEST_DATA_REG   : 0xA420000C;

declare const MI_INIT_MODE_REG       : 0xA4300000;
declare const MI_MODE_REG            : 0xA4300000;
declare const MI_VERSION_REG         : 0xA4300004;
declare const MI_NOOP_REG            : 0xA4300004;
declare const MI_INTR_REG            : 0xA4300008;
declare const MI_INTR_MASK_REG       : 0xA430000C;

declare const VI_STATUS_REG          : 0xA4400000;
declare const VI_CONTROL_REG         : 0xA4400000;
declare const VI_ORIGIN_REG          : 0xA4400004;
declare const VI_DRAM_ADDR_REG       : 0xA4400004;
declare const VI_WIDTH_REG           : 0xA4400008;
declare const VI_H_WIDTH_REG         : 0xA4400008;
declare const VI_INTR_REG            : 0xA440000C;
declare const VI_V_INTR_REG          : 0xA440000C;
declare const VI_CURRENT_REG         : 0xA4400010;
declare const VI_V_CURRENT_LINE_REG  : 0xA4400010;
declare const VI_BURST_REG           : 0xA4400014;
declare const VI_TIMING_REG          : 0xA4400014;
declare const VI_V_SYNC_REG          : 0xA4400018;
declare const VI_H_SYNC_REG          : 0xA440001C;
declare const VI_LEAP_REG            : 0xA4400020;
declare const VI_H_SYNC_LEAP_REG     : 0xA4400020;
declare const VI_H_START_REG         : 0xA4400024;
declare const VI_H_VIDEO_REG         : 0xA4400024;
declare const VI_V_START_REG         : 0xA4400028;
declare const VI_V_VIDEO_REG         : 0xA4400028;
declare const VI_V_BURST_REG         : 0xA440002C;
declare const VI_X_SCALE_REG         : 0xA4400030;
declare const VI_Y_SCALE_REG         : 0xA4400034;

declare const AI_DRAM_ADDR_REG       : 0xA4500000;
declare const AI_LEN_REG             : 0xA4500004;
declare const AI_CONTROL_REG         : 0xA4500008;
declare const AI_STATUS_REG          : 0xA450000C;
declare const AI_DACRATE_REG         : 0xA4500010;
declare const AI_BITRATE_REG         : 0xA4500014;

declare const PI_DRAM_ADDR_REG       : 0xA4600000;
declare const PI_CART_ADDR_REG       : 0xA4600004;
declare const PI_RD_LEN_REG          : 0xA4600008;
declare const PI_WR_LEN_REG          : 0xA460000C;
declare const PI_STATUS_REG          : 0xA4600010;
declare const PI_BSD_DOM1_LAT_REG    : 0xA4600014;
declare const PI_BSD_DOM1_PWD_REG    : 0xA4600018;
declare const PI_BSD_DOM1_PGS_REG    : 0xA460001C;
declare const PI_BSD_DOM1_RLS_REG    : 0xA4600020;
declare const PI_BSD_DOM2_LAT_REG    : 0xA4600024;
declare const PI_BSD_DOM2_PWD_REG    : 0xA4600028;
declare const PI_BSD_DOM2_PGS_REG    : 0xA460002C;
declare const PI_BSD_DOM2_RLS_REG    : 0xA4600030;

declare const RI_MODE_REG            : 0xA4700000;
declare const RI_CONFIG_REG          : 0xA4700004;
declare const RI_CURRENT_LOAD_REG    : 0xA4700008;
declare const RI_SELECT_REG          : 0xA470000C;
declare const RI_REFRESH_REG         : 0xA4700010;
declare const RI_COUNT_REG           : 0xA4700010;
declare const RI_LATENCY_REG         : 0xA4700014;
declare const RI_RERROR_REG          : 0xA4700018;
declare const RI_WERROR_REG          : 0xA470001C;

declare const SI_DRAM_ADDR_REG       : 0xA4800000;
declare const SI_PIF_ADDR_RD64B_REG  : 0xA4800004;
declare const SI_PIF_ADDR_WR64B_REG  : 0xA4800010;
declare const SI_STATUS_REG          : 0xA4800018;

declare const mem: MemModule;
declare const events: EventsModule;
declare const debug: DebugModule;
declare const asm: ASMModule;
declare const console: ConsoleModule;
declare const fs: FSModule;
declare const cpu: CPUModule;

declare function alert(message: string, caption?: string): number;

interface Number {
    hex(length?: number): string;
}


