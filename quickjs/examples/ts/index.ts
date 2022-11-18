import { R2Pipe, R2Api } from "./r2pipe";
import { refsTest } from "./examples";

export function main(r2: R2Pipe) {
	// testProgram(r2);
	// newTestProgram(r2);
	// refsTest(r2);
	r2.log("Hello World");
	const api = new R2Api(r2);
	api.analyzeProgram();
	const functions = api.listFunctions();
	r2.log("FunctionCount "+ functions.length);

	const bbs = api.ptr("main").functionBasicBlocks();
	for (let bb of bbs) {
		const op = api.ptr(bb.addr).instruction();
		r2.log(api.hex(bb.addr) + " " + op.opcode);
	}
}
