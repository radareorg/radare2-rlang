import os
import json
// import x.json2


struct Core {
	t_pe string [json: @type]
	file string
}

struct Info {
	core Core
	foo string
}

fn file_type(r2 &R2) string {
	mut s := r2.cmd('ij')
	y := json.decode(Info, s) or {
		return 'xx'
	}
	return y.core.file
}

fn file_typ(r2 &R2)string{
/*
	mut s := r2.cmd('ij')
	y := json2.raw_decode(s) or {
		return 'xx'
	}
*/
	return 'jeje'
	// return y.as_map()['core'].as_map()['t_pe'].str()
}

pub fn entry(r2 &R2) {
	arg := file_type(r2)
	println('File: ' + arg)
	arq := file_typ(r2)
	println('File2: '+arq)
	println(r2.cmd('?E hello'))
	os.system('echo world')
}
