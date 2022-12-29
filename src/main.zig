const std = @import("std");

const libuv = @import("libuv");

/// Run the program
pub fn main() !void {
    // Initialize the loop
    var loop = try libuv.Loop.init(std.heap.page_allocator);
    defer loop.deinit();
    // Run the loop
    try loop.run(libuv.Loop.RunMode.DEFAULT);
    // Close the loop
    try loop.close();
}
