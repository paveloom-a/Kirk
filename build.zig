const std = @import("std");

const deps = @import("deps.zig");

pub fn build(b: *std.build.Builder) !void {
    // Add standard target options
    const target = b.standardTargetOptions(.{});
    // Add standard release options
    const mode = b.standardReleaseOptions();
    // Add the executable
    const exe = b.addExecutable("groovy", "src/main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.install();
    // Use the `stage1` compiler because of
    // https://github.com/ziglang/zig/issues/12325
    exe.use_stage1 = true;
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
    inline for (@typeInfo(deps.package_data).Struct.decls) |decl| {
        const pkg = @field(deps.package_data, decl.name);
        // Add the include paths
        inline for (pkg.c_include_dirs) |path| {
            exe.addIncludePath(@field(deps.dirs, decl.name) ++ "/" ++ path);
        }
        // Add the C source files
        inline for (pkg.c_source_files) |path| {
            exe.addCSourceFile(@field(deps.dirs, decl.name) ++ "/" ++ path, pkg.c_source_flags);
        }
    }
    exe.linkLibC();
}
