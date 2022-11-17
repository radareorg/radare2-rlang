export interface R2Pipe {
    cmd(cmd: string): string;
    cmdj(cmd: string): object;
    log(msg: string): string;
}
export declare class R2Api {
    r2: R2Pipe;
    constructor(r2: R2Pipe);
    ptr(s: string): NativePointer;
    cmd(s: string): string;
    cmdj(s: string): string;
}
export declare class NativePointer {
    addr: string;
    api: R2Api;
    constructor(api: R2Api, s: string);
    readByteArray(len: number): any;
    add(a: number): void;
    sub(a: number): void;
    readCString(): any;
}
