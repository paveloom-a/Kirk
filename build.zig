const std = @import("std");

const deps = @import("deps.zig");

pub fn build(b: *std.build.Builder) !void {
    // Add standard target options
    const target = b.standardTargetOptions(.{});
    // Add standard release options
    const mode = b.standardReleaseOptions();
    // Add the `libuv` library
    const lib = b.addStaticLibrary("libuv", "src/libuv/lib.zig");
    lib.setBuildMode(mode);
    lib.install();
    // Add the unit tests
    const unit_tests_step = b.step("test", "Run the unit tests");
    const unit_tests = b.addTest("src/libuv/lib.zig");
    unit_tests.setBuildMode(mode);
    unit_tests_step.dependOn(&unit_tests.step);
    unit_tests.test_evented_io = true;
    // Add the executable
    const exe = b.addExecutable("groovy", "src/main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.install();
    // Add a run step for the executable
    const run_step = b.step("run", "Run");
    {
        const run_cmd = exe.run();
        run_cmd.step.dependOn(b.getInstallStep());
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }
        run_step.dependOn(&run_cmd.step);
    }
    // Add the dependencies
    inline for (.{ exe, lib, unit_tests }) |step| {
        inline for (@typeInfo(deps.package_data).Struct.decls) |decl| {
            const pkg = @field(deps.package_data, decl.name);
            // Add the include paths
            inline for (pkg.c_include_dirs) |path| {
                step.addIncludePath(@field(deps.dirs, decl.name) ++ "/" ++ path);
            }
            // Add the C source files
            inline for (pkg.c_source_files) |path| {
                step.addCSourceFile(@field(deps.dirs, decl.name) ++ "/" ++ path, pkg.c_source_flags);
            }
        }
        // Link the C library
        step.linkLibC();
        // Use the `stage1` compiler because of
        // https://github.com/ziglang/zig/issues/12325
        step.use_stage1 = true;
    }
    // Add the packages
    const libuv_pkg = std.build.Pkg{
        .name = "libuv",
        .source = .{ .path = "src/libuv/lib.zig" },
        .dependencies = &[_]std.build.Pkg{},
    };
    exe.addPackage(libuv_pkg);
}
