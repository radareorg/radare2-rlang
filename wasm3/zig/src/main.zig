const wasm3 = @import("wasm3");
const std = @import("std");
// const r2 = @import("r2.zig");

const kib = 1024;
const mib = 1024 * kib;
const gib = 1024 * mib;

fn r2pipe(core: *u8, cmd: *u8) void {
    _ = core;
    _ = cmd;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();

    var a = gpa.allocator();

    //    var args = try std.process.argsAlloc(a);
    //    defer std.process.argsFree(a, args);
    //
    //    if (args.len < 2) {
    //        std.log.err("Please provide a wasm file on the command line!\n", .{});
    //        std.os.exit(1);
    //    }
    //  std.log.info("Loading wasm file {s}!\n", .{args[1]});

    var env = wasm3.Environment.init();
    defer env.deinit();

    var rt = env.createRuntime(16 * kib, null);

    defer rt.deinit();
    errdefer rt.printError();
    var file = "hello.wasm";

    var mod_bytes = try std.fs.cwd().readFileAlloc(a, file, 512 * kib);
    defer a.free(mod_bytes);
    var mod = try env.parseModule(mod_bytes);
    try rt.loadModule(mod);
    // try mod.linkRawFunction("r2", "cmd", "v(ss)", @ptrCast(*const anyopaque, &r2pipe));
    try mod.linkWasi();
    //    if (true) {
    //      try mod.linkLibrary("r2", struct {
    //          pub inline fn add(_: *std.mem.Allocator, lh: i32, rh: i32) i32 {
    //              return lh + rh;
    //          }
    //          pub inline fn sub(_: *std.mem.Allocator, lh: i32, rh: i32) i32 {
    //              return lh - rh;
    //          }
    //      }, &a);
    //    }

    var start_fn = try rt.findFunction("main");
    std.log.info("finding {s}", .{"main"});
    start_fn.call(void, .{}) catch |e| switch (e) {
        error.TrapExit => {},
        else => return e,
    };
}
