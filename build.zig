const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardOptimizeOption(.{});

    const module = b.addModule("cpp_ray", .{
        .target = target,
        .optimize = mode,
        .link_libc = true,
        .link_libcpp = true
    });

    module.addCSourceFile(.{
        .file = b.path("main.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });
    module.addCSourceFile(.{
        .file = b.path("dtypes.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });
    module.addCSourceFile(.{
        .file = b.path("otree/otree.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });
    module.addCSourceFile(.{
        .file = b.path("otree/otree_ray.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });
    module.addCSourceFile(.{
        .file = b.path("game_data.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });
    module.addCSourceFile(.{
        .file = b.path("vox_render.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });
    module.addCSourceFile(.{
        .file = b.path("render_shader.cpp"),
        .language = .cpp,
        // .flags = &[_][]const u8{"-std=c11"},
    });

    const exe = b.addExecutable(.{
        .name = "cpp_raylib",
        .root_module = module,
    });

    exe.root_module.linkSystemLibrary("raylib", .{});

    b.installArtifact(exe);

    const run_exe = b.addRunArtifact(exe);

    const step = b.step("run", "run");
    step.*.dependOn(&run_exe.*.step);
}
