const std = @import("std");

/// Run the program
pub fn main() void {
    // Import the `libuv` library
    const c = @cImport({
        @cInclude("uv.h");
    });
    // Initialize the loop
    var loop: *c.uv_loop_t = c.uv_default_loop();
    defer _ = c.uv_loop_close(loop);
    // Run the loop
    std.debug.print("Using the default loop.\n", .{});
    _ = c.uv_run(loop, c.UV_RUN_DEFAULT);
}
