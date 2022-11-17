
export interface R2Pipe {
	cmd(cmd: string): string;
	cmdj(cmd: string): object;
	log(msg: string): string;
}

export class R2Api {
	public r2: R2Pipe;

	constructor(r2: R2Pipe) {
		this.r2 = r2;
	}

	ptr(s: string): NativePointer {
		this.r2.log("new ptr");
		return new NativePointer(this, s);
	}
	cmd(s: string): string {
		return this.r2.cmd(s);
	}
}

export class NativePointer {
	addr: string;
	api: R2Api;
	constructor(api: R2Api, s: string) {
		this.api = api;
		api.cmd("?e x");
		this.addr = s;
	}
	readByteArray(len: number) {
		return JSON.parse(this.api.cmd(`p8j ${len}@${this.addr}`));
	}
	add(a: number) {
		this.addr = this.api.cmd(`?v ${this.addr} + ${a}`);
	}
	sub(a: number) {
		this.addr = this.api.cmd(`?v ${this.addr} - ${a}`);
	}
	readCString() {
		return JSON.parse(this.api.cmd(`psj@${this.addr}`)).string;
	}
}
