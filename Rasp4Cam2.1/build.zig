const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .opengl_version = .gl_2_1,
    });

    const exe = b.addExecutable(.{
        .name = "zig_test",
        //.root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
    });
    exe.linkLibC();
    
    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "src/main.c",
            "src/device_wifi_interface_pi.c",
            "src/benchmark.c",
            "src/cam.c",
            "src/mem_manager.c"
        },
        .flags = &[_][]const u8{
            "-std=c11",
        },
    });
    
    exe.addIncludePath(.{ .path = "include" });
    exe.linkLibrary(raylib_dep.artifact("raylib"));

    exe.addIncludePath(.{ .path = "../Operators" });
    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "../Operators/operators.c",
            "../Operators/operators_basic.c",
            "../Operators/operators_rgb565.c",
            "../Operators/operators_rgb888.c",
        },
        .flags = &[_][]const u8{
            "-std=c11",
        },
    });
    //exe.defineCMacro("USE_MEM_MANAGER", "");

    b.installArtifact(exe);
    
    const run_cmd = b.addRunArtifact(exe);
    
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
