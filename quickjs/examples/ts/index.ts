
import { R2Pipe, R2Api, NativePointer } from "./r2pipe";

// main

function main(r2: R2Pipe) {
	var api = new R2Api(r2);
	const p = api.ptr("0x80000");
	const bytes = p.readByteArray(32);
	p.add(4);
	r2.log(JSON.stringify(bytes));

	const $ptr = api.ptr;
	const $ = r2.cmd;

	const text = $ptr("entry0").readCString();
	r2.log(`Hello World ${text}`)
	r2.log(text);
	r2.log(JSON.stringify(Object.keys(r2)));
	r2.log("done");
}
