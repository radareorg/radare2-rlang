// Example V core plugin registration
// Usage:
//   r2 -qi vlang/tests/test-core-vlang.v -
// Then type 'vhello' in the r2 prompt to test.

pub fn vcore_call(core &R2, input byteptr) bool {
    unsafe {
        s := input.vstring()
        if s == 'vhello' {
            println('Hello from V core plugin!')
            println(core.cmd('?E Hello V'))
            return true
        }
    }
    return false
}

pub fn entry(core &R2) {
    // name, callback, desc, license
    _ = core.register_core('vcoretest', vcore_call, 'Example core plugin written in V', 'MIT')
}

