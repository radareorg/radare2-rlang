import { R2Pipe, R2Api } from "./r2pipe";

function main(r2: R2Pipe) {
	const a = new R2Api(r2)
	const regs = a.getRegisters();
	console.log("Hello World");
}
