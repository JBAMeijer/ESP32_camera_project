const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .shared = true,
    });

    const exe = b.addExecutable(.{
        .name = "main",
        .target = target,
        .optimize = optimize,
    });
    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "src/main.c",
            "src/benchmark.c",
            "src/device_wifi_interface.c",
        },
        .flags = &[_][]const u8{
            "-std=c11",
        },
    });
    // exe.addCSourceFile(.{
    //     .file = .{ .path = "src/main.c" },
    //     .flags = &[_][]const u8{"-std=c11"},
    // });
    exe.addIncludePath(.{ .path = "include" });
    exe.linkLibrary(raylib_dep.artifact("raylib"));
    var gen_step = b.addWriteFiles();
    exe.step.dependOn(&gen_step.step);

    const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
    exe.addCSourceFile(.{ .file = raygui_c_path });
    exe.addIncludePath(.{ .path = "libs/raygui/src" });
    exe.installHeader(.{ .path = "libs/raygui/src/raygui.h" }, "raygui.h");

    exe.addIncludePath(.{ .path = "libs/raygui/styles/dark" });
    exe.installHeader(.{ .path = "libs/raygui/styles/dark/style_dark.h" }, "style_dark.h");

    b.installArtifact(exe);

    const run_cmd = b.addRunArtifact(exe);

    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
