import os

pub fn entry(core &R2) {
	println("hello")
	println(core.cmd('?E hello'))
	os.system('echo world')
}
