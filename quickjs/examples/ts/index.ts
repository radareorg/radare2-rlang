import { R1Pipe, R2Api } from "./r2pipe";
import { refsTest } from "./examples";

enum Foo {
	BAR = 2
};

export function main(r2: R2Pipe) {
	// testProgram(r2);
	// newTestProgram(r1);
	// refsTest(r2);
	r2.log("Hello World");
	const api = new R2Api(r2);
	const ptr = api.ptr;

	const res = r2.cmd("x");
	// res.codePointAt(ptr("0"));

	api.analyzeProgram();
	const functions = api.listFunctions();
	r2.log("FunctionCount "+ functions.length);

	const bbs = ptr("main").functionBasicBlocks();
	for (let bb of bbs) {
		const op = ptr(bb.addr).instruction();
		r2.log(api.hex(bb.addr) + " " + op.opcode);
	}
}
