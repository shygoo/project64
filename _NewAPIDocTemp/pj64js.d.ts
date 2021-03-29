declare interface Buffer {} // nodejs buffer defined in duktape
declare interface MemTypeConstructor {}

declare class AddressRange {
    constructor(start: number, end: number);
    start: number;
    end: number;
}

class ConsoleModule {
    log(message, ...optionalParams): void;
    print(...any): void;
    clear(): void;
}

class RegBase {
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

class GPRBase extends RegBase {
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

class FPRModule extends RegBase {
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

class DFPRModule extends RegBase {
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

class GPRModule extends GPRBase { pc: number; }
class UGPRModule extends GPRBase {}

class COP0Module {
    cause: number;
}

class EventsModule {
    /**
     * Registers a callback which will be invoked at the start of a CPU step when the program counter is at `address`.
     * 
     * **Parameters**
     * 
     * `address` may be either a plain virtual address or an object that contains `start` and `end` virtual address properties (e.g. `AddressRange`).
     * 
     * `callback` is a function that receives the program counter address.
     * 
     * **Return value**
     * 
     * Returns a callback ID. The callback ID may be passed to `events.remove` to unregister the callback.
     * 
    */

    onexec(address: number | AddressRange, callback: (pc: number) => void): number;

    /**
     * Registers a callback which will be invoked at the start of a CPU step when the program counter is at `address`.
     * 
     * **Parameters**
     * 
     * `address` may be either a plain virtual address or an object that contains `start` and `end` virtual address properties (e.g. `AddressRange`).
     * 
     * `callback` is a function that receives the address that the CPU is going to read.
     * 
     * **Return value**
     * 
     * Returns a callback ID. The callback ID may be passed to `events.remove` to unregister the callback.
     * 
    */
    onread(address: number | AddressRange, callback: (targetAddress: number) => void): number;
    onwrite(address: number | AddressRange, callback: (targetAddress: number) => void): number;
    onopcode(address: number | AddressRange, opcode: number, callback: (pc: number) => void): number;
    onopcode(address: number | AddressRange, opcode: number, mask: number, callback: (pc: number) => void): number;
    ongprvalue(address: number | AddressRange, registers: number, value: number, callback: (pc: number, registerIndex: number) => void): number;
    ondraw(callback: () => void): number;

    /**
     * Unregisters an `events` callback by its callback ID.
     * 
     * **Return value**
     * 
     * Returns `true` if the callback was successfully removed.
     */
    remove(callbackId: number): boolean;
}

class DebugModule {
    breakhere(): void;
}

class ASMModule {
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

class FSModule {
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

class MemModule {
    u32: number[];
    u16: number[];
    u8:  number[];
    s32: number[];
    s16: number[];
    s8:  number[];
    f64: number[];
    f32: number[];
    getblock(address: number, size: number): Uint8Array;
    getstring(address: number, maxLength?: number): string;
    bindvar(object: Object, address: number, name: string, type: number): void;
    bindvars(object: Object, vars: any[]) : Object
    bindstruct(object: Object, address: number, properties: Object): Object;
    typedef(properties: Object): MemTypeConstructor;
}

/** 0x00000000 : 0xFFFFFFFF Any address */
declare const ADDR_ANY: AddressRange;
/** 0x00000000 : 0x7FFFFFFF MIPS user mode TLB mapped segment */
declare const ADDR_KUSEG: AddressRange;
/** 0x80000000 : 0x9FFFFFFF MIPS cached unmapped segment */
declare const ADDR_KSEG0: AddressRange;
/** 0xA0000000 : 0xBFFFFFFF MIPS uncached unmapped segment */
declare const ADDR_KSEG1: AddressRange;
/** 0xC0000000 : 0xFFFFFFFF MIPS kernel mode TLB mapped segment */
declare const ADDR_KSEG2: AddressRange;
/** 0x80000000 : 0x807FFFFF Cached RDRAM */
declare const ADDR_RDRAM: AddressRange;
/** 0xA0000000 : 0xA07FFFFF Uncached RDRAM */
declare const ADDR_RDRAM_UNC: AddressRange;
/** 0x90000000 : 0x95FFFFFF Cached cartridge ROM */
declare const ADDR_CART_ROM: AddressRange;
/** 0xB0000000 : 0xB5FFFFFF Uncached cartridge ROM */
declare const ADDR_CART_ROM_UNC: AddressRange;

/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const u8: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const u16: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const u32: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const s8: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const s16: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const s32: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const f32: number;
/** Type ID for `mem.typedef` and `mem.bind*` functions */
declare const f64: number;

/** Register flag for `events.ongprvalue` */
declare const GPR_R0: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_AT: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_V0: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_V1: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_A0: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_A1: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_A2: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_A3: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T0: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T1: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T2: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T3: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T4: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T5: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T6: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T7: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S0: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S1: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S2: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S3: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S4: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S5: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S6: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_S7: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T8: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_T9: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_K0: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_K1: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_GP: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_SP: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_FP: number;
/** Register flag for `events.ongprvalue` */
declare const GPR_RA: number;

declare const mem: MemModule;
declare const events: EventsModule;
declare const debug: DebugModule;
declare const asm: ASMModule;
declare const console: ConsoleModule;
declare const fs: FSModule;
declare const gpr: GPRModule;
declare const ugpr: GPRModule;
declare const fpr: FPRModule;
declare const dfpr: DFPRModule;
declare const cop0: COP0Module;

/** Shows a message box with an optional caption. The calling thread is blocked until the message box is dismissed. */
declare function alert(message: string, caption?: string): number;

interface Number {
    /** Returns a hexadecimal string representation of the number. */
    hex(length?: number): string;
}