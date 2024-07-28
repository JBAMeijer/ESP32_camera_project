const std = @import("std");

pub fn build(b: *std.Build) !void {
    const n_data_blocks_required = b.option(u32, "n_data_blocks_required", "Set the required amount of data blocks that will be used by the mem manager") orelse 20;
    //const n_data_blocks_for_viewer = b.option(u32, "n_data_blocks_for_viewer", "Set the required amount of data blocks used by the viewer") orelse 10;
    //const n_data_blocks_for_queue = b.option(u32, "n_data_blocks_for_queue", "Set the required amount of data blocks used by the queue") orelse 10;
    //const n_data_blocks_for_processing = b.option(u32, "n_data_blocks_for_processing", "Set the required amount of data blocks used for vision processing") orelse 10;
    
    //const n_data_blocks_required = 50;
    
    //if(n_data_blocks_for_viewer > n_data_blocks_required) {
    //    std.debug.print("ERROR: viewer data blocks can't be more then the required amount!'", .{});
    //    return error.VIEWER_BLOCKS_EXCEEDED;
    //}
    
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
    
    exe.defineCMacro("RASP4", "");
    
    exe.addCSourceFiles(.{
        .files = &[_][]const u8{
            "src/main.c",
            "src/device_wifi_interface_pi.c",
            "src/benchmark.c",
            "src/cam.c",
            "src/mem_manager.c",
            //"src/vision.c",
        },
        .flags = &[_][]const u8{
            "-std=c11",
            "-Wall",
            "-Werror",
        },
    });
    
    exe.addIncludePath(.{ .path = "include" });
    exe.linkLibrary(raylib_dep.artifact("raylib"));
    var gen_step = b.addWriteFiles();
    exe.step.dependOn(&gen_step.step);
    
    const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
    exe.addCSourceFile(.{ .file = raygui_c_path });
    exe.addIncludePath(.{ .path = "../submodules/raygui/src" });
    exe.installHeader(.{ .path = "../submodules/raygui/src/raygui.h" }, "raygui.h");
    
    exe.linkSystemLibrary("pigpio");

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
            "-fno-sanitize=undefined",
        },
    });
    
    exe.defineCMacro("IMAGE_RES_VGA", "");

    var buf: [10]u8 = undefined;
    const str = try std.fmt.bufPrint(&buf, "{}", .{n_data_blocks_required});
    exe.defineCMacro("N_REQUIRED_DATA_BLOCKS", str);
    
    //str = try std.fmt.bufPrint(&buf, "{}", .{n_data_blocks_for_viewer});
    //exe.defineCMacro("N_REQUIRED_DATA_BLOCKS_VIEWER", str);
    
    //str = try std.fmt.bufPrint(&buf, "{}", .{n_data_blocks_for_queue});
    //exe.defineCMacro("N_REQUIRED_DATA_BLOCKS_QUEUE", str);
    //exe.defineCMacro("IMAGE_RES_VGA", "");

    b.installArtifact(exe);
    
    const run_cmd = b.addRunArtifact(exe);
    
    run_cmd.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
