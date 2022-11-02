const std = @import("std");
pub const wasm3_build = @import("libs/zig-wasm3/submod_build_plugin.zig");

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    // const target = std.zig.CrossTarget { .cpu_arch = .wasm32, .os_tag = .wasi, };
    const mode = b.standardReleaseOptions();
    const exe = b.addExecutable("hello", "src/main.zig");
    exe.setTarget(target);
    wasm3_build.addTo(exe, "libs/wasm3");
    exe.addPackage(wasm3_build.pkg(null));
    exe.setBuildMode(mode);
    exe.install();
}
