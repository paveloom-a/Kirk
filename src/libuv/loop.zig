const std = @import("std");

const c = @import("c.zig").c;

const check = @import("error.zig").check;

/// An event loop
pub const Loop = struct {
    const Self = @This();
    /// Mode used to run the loop with
    pub const RunMode = enum(c_uint) {
        DEFAULT = c.UV_RUN_DEFAULT,
        ONCE = c.UV_RUN_ONCE,
        NOWAIT = c.UV_RUN_NOWAIT,
    };
    /// A wrapped `libuv`'s event loop
    loop: *c.uv_loop_t,
    /// An allocator
    allocator: std.mem.Allocator,
    /// Initialize the loop
    pub fn init(allocator: std.mem.Allocator) !Self {
        // Allocate the memory for the loop
        var loop = try allocator.create(c.uv_loop_t);
        // Initialize the loop
        try check(c.uv_loop_init(loop));
        // Initialize itself
        return Self{
            .loop = loop,
            .allocator = allocator,
        };
    }
    /// Run the loop
    pub fn run(self: *Self, run_mode: RunMode) !void {
        try check(c.uv_run(self.loop, @enumToInt(run_mode)));
    }
    /// Close the loop
    pub fn close(self: *Self) !void {
        try check(c.uv_loop_close(self.loop));
    }
    /// Free the memory allocated to the loop
    pub fn deinit(self: *Self) void {
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
    try loop.run(Loop.RunMode.DEFAULT);
    // Close the loop
    try loop.close();
}
