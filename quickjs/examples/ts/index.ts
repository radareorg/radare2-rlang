
import { R2Pipe, R2Api, NativePointer, SearchResult } from "./r2pipe";

// main

function firstString(api: R2Api) : NativePointer {
	return api.ptr(api.r2.cmd("f,name/str/str.,addr/cols/name,name/head/1,:quiet"))
	// return api.ptr(api.r2.cmd("f~str.[:0]").split(' ')[0])
}

export function main(r2: R2Pipe) {
	// testProgram(r2);
	// newTestProgram(r2);
	refsTest(r2);
}

function refsTest(r2: R2Pipe) {
	const api = new R2Api(r2);
	api.analyzeProgram();
	const refsToExit = api.ptr("sym.imp.exit").xrefs();
	for (let ref of refsToExit) {
		r2.log("--> " + api.hex(ref.from) + " : " + ref.opcode);
	}
}

function newTestProgram(r2: R2Pipe) {
	const api = new R2Api(r2);
	api.clearScreen();
	api.analyzeProgram();
	api.cmd("aeim");
	api.setRegisters({PC: "entry0"});
	r2.log("PC " + api.getRegisters()["pc"]);
	api.step();
	r2.log("PC " + api.getRegisters()["pc"]);
	api.step();
	r2.log("PC " + api.getRegisters()["pc"]);
	const results : SearchResult[] = api.searchString("lib");
	for (let res of results) {
		r2.log("--> " + res.data);
		const count = api.math("entry0 + 32");
		r2.log("count = "+count);
	}
}

function testProgram(r2: R2Pipe) {
	var api = new R2Api(r2);
	api.clearScreen();

	const p = api.ptr("entry0");
	p.analyzeFunction();
	r2.log("===");
	r2.log("===");
	r2.log("MY NAME IS " + p.name());
	const bytes = api.ptr("entry0").readByteArray(32);
	p.add(4);
	
	r2.log("bytes: " + JSON.stringify(bytes));

	const op = api.ptr("entry0").instruction();
	r2.log("op size = " + op.size);

	const text = firstString(api).readCString();
	r2.log(`First string found is:\n\t- ${text}`)

	const op1 = p.instruction();
	r2.log("OP1: " + op.opcode);

	p.add(op.size);
	r2.log(JSON.stringify(op1.opex, null, 2));

	const op2 = p.instruction();
	r2.log("OP2: " + op2.opcode);

	const bb = p.basicBlock();
	for (let b of bb.instrs) {
		const bp = api.ptr(b).instruction();
		r2.log("" + bp.addr + " " + bp.opcode);
	}
}
