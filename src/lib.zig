const std = @import("std");

// Import the `libuv` library
const c = @cImport({
    @cInclude("uv.h");
});

/// An event loop
pub const Loop = struct {
    const Self = @This();
    /// A wrapped `libuv`'s event loop
    loop: *c.uv_loop_t,
    /// An allocator
    allocator: std.mem.Allocator,
    /// Initialize the loop
    pub fn init(allocator: std.mem.Allocator) !Self {
        // Allocate the memory for the loop
        var loop = try allocator.create(c.uv_loop_t);
        // Initialize the loop
        _ = c.uv_loop_init(loop);
        // Initialize itself
        return Self{
            .loop = loop,
            .allocator = allocator,
        };
    }
    /// Run the loop
    pub fn run(self: *Self) void {
        _ = c.uv_run(self.loop, c.UV_RUN_DEFAULT);
    }
    /// Deinitialize the loop
    pub fn deinit(self: *Self) void {
        // Close the loop
        _ = c.uv_loop_close(self.loop);
        // Free the memory
        self.allocator.destroy(self.loop);
        self.* = undefined;
    }
};

// Test an empty loop for memory leaks
test "Empty loop" {
    // Initialize the loop
    var loop = try Loop.init(std.testing.allocator);
    defer loop.deinit();
    // Run the loop
    loop.run();
}
