import json
import x.json2

fn (r2 &R2)once(k string) bool {
	if r2.cmd('fd once.$k') != '' {
		return true
	}
	r2.cmd('f once.$k=-123')
	return false
}

fn (r2 &R2)main() ? {
	arg := file_type(r2)
	println('File: ' + arg)
	arq := file_typ(r2)
	println('File2: '+arq)
	println(r2.cmd('?E hello'))
	r2.cmd('aaa')
	addrs := r2.function_addrs()?
	for at in addrs {
		fcn := r2.function_info(at)?
		println('--- ${fcn.name}')
	}

	fcns := r2.functions()?
	for fcn in fcns {
		fcnmap := fcn.as_map()
		name := fcnmap['name']
		addr := fcnmap['offset']
		println('$addr  $name')
	}
}

pub fn entry(r2 &R2) {
	if r2.once('') {
		eprintln('dont run this script twice')
		return
	}
	r2.main() or {
		panic(err)
	}
}

// helper functions

struct Core {
	t_pe string @[json: @type]
	file string
}

struct Function {
	addr u64
	name string
}

struct Info {
	core Core
	foo string
}

fn (f Function)str() string {
	return '$f.addr  $f.name'
}

fn file_type(r2 &R2) string {
	mut s := r2.cmd('ij')
	y := json.decode(Info, s) or {
		return 'xx'
	}
	return y.core.file
}

fn (r2 &R2)function_addrs() ?[]u64 {
	mut s := r2.cmd('aflqj')
	y := json2.raw_decode(s)?
	return y.arr().map(u64(it.i64()))
}

fn (r2 &R2)function_info(addr u64) ?Function {
	eprintln('afij @ ${addr}')
	mut s := r2.cmd('afij @ ${addr}')
	y := json2.raw_decode(s)?
	a := y.arr()
	for item in a {
		f := item.as_map()
		return Function {
			name: f['name'].str()
			addr: addr
		}
	}
	return error('Cannot find function')
}

fn (r2 &R2)functions() ?[]json2.Any {
	mut s := r2.cmd('aflj')
	y := json2.raw_decode(s)?
	return y.arr() // map()['core'].as_map()['t_pe'].str()
}

fn file_typ(r2 &R2)string{
	mut s := r2.cmd('ij')
	y := json2.raw_decode(s) or {
		return 'xx'
	}
	return y.as_map()['core'].as_map()['t_pe'].str()
}

