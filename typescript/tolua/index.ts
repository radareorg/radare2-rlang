import { R2Pipe, R2Api } from "./r2pipe";

declare function inspect(o: any): string;

export function main(r2: R2Pipe) {
	console.log(r2)
	const a = new R2Api(r2)
	// console.log(inspect(a))
	r2.cmd("aeim");
// 	console.log(r2.cmd("i"))
 	const regs = a.getRegisters();
	console.log(inspect(regs)); // 
	console.log("Hello World");
}
