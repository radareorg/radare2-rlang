const std = @import("std");

extern "r2" fn add(a: i32, b: i32, mul: *i32) i32;

fn main() void {
    const add_res = add(a1, a2, &mul_res);
    std.info.log("Hello World", .{add_res});
}
