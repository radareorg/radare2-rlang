
import { R2Pipe, R2Api, NativePointer } from "./r2pipe";

// main

function firstString(api: R2Api) : NativePointer {
	return api.ptr(api.r2.cmd("f,name/str/str.,addr/cols/name,name/head/1,:quiet"))
	// return api.ptr(api.r2.cmd("f~str.[:0]").split(' ')[0])
}

export function main(r2: R2Pipe) {
	var api = new R2Api(r2);
	const p = api.ptr("entry0");
	const bytes = api.ptr("entry0").readByteArray(32);
	p.add(4);
	r2.log("bytes: " + JSON.stringify(bytes));

	const text = firstString(api).readCString();
	r2.log(`First string found is:\n\t- ${text}`)
}
