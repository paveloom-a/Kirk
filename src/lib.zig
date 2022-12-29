const std = @import("std");

// Import the `libuv` library
const c = @cImport({
    @cInclude("uv.h");
});

/// A `libuv` error
pub const Error = error{
    /// Argument list too long
    E2BIG,
    /// Permission denied
    EACCES,
    /// Address already in use
    EADDRINUSE,
    /// Address not available
    EADDRNOTAVAIL,
    /// Address family not supported
    EAFNOSUPPORT,
    /// Resource temporarily unavailable
    EAGAIN,
    /// Address family not supported
    EAI_ADDRFAMILY,
    /// Temporary failure
    EAI_AGAIN,
    /// Bad `ai_flags` value
    EAI_BADFLAGS,
    /// Invalid value for hints
    EAI_BADHINTS,
    /// Request canceled
    EAI_CANCELED,
    /// Permanent failure
    EAI_FAIL,
    /// `ai_family` not supported
    EAI_FAMILY,
    /// Out of memory
    EAI_MEMORY,
    /// No address
    EAI_NODATA,
    /// Unknown node or service
    EAI_NONAME,
    /// Argument buffer overflow
    EAI_OVERFLOW,
    /// Resolved protocol is unknown
    EAI_PROTOCOL,
    /// Service not available for socket type
    EAI_SERVICE,
    /// Socket type not supported
    EAI_SOCKTYPE,
    /// Connection already in progress
    EALREADY,
    /// Bad file descriptor
    EBADF,
    /// Resource busy or locked
    EBUSY,
    /// Operation canceled
    ECANCELED,
    /// Invalid Unicode character
    ECHARSET,
    /// Software caused connection abort
    ECONNABORTED,
    /// Connection refused
    ECONNREFUSED,
    /// Connection reset by peer
    ECONNRESET,
    /// Destination address required
    EDESTADDRREQ,
    /// File already exists
    EEXIST,
    /// Bad address in system call argument
    EFAULT,
    /// File too large
    EFBIG,
    /// Host is unreachable
    EHOSTUNREACH,
    /// Interrupted system call
    EINTR,
    /// Invalid argument
    EINVAL,
    /// I/O error
    EIO,
    /// Socket is already connected
    EISCONN,
    /// Illegal operation on a directory
    EISDIR,
    /// Too many symbolic links encountered
    ELOOP,
    /// Too many open files
    EMFILE,
    /// Message too long
    EMSGSIZE,
    /// Name too long
    ENAMETOOLONG,
    /// Network is down
    ENETDOWN,
    /// Network is unreachable
    ENETUNREACH,
    /// File table overflow
    ENFILE,
    /// No buffer space available
    ENOBUFS,
    /// No such device
    ENODEV,
    /// No such file or directory
    ENOENT,
    /// Not enough memory
    ENOMEM,
    /// Machine is not on the network
    ENONET,
    /// Protocol not available
    ENOPROTOOPT,
    /// No space left on device
    ENOSPC,
    /// Function not implemented
    ENOSYS,
    /// Socket is not connected
    ENOTCONN,
    /// Not a directory
    ENOTDIR,
    /// Directory not empty
    ENOTEMPTY,
    /// Socket operation on non-socket
    ENOTSOCK,
    /// Operation not supported on socket
    ENOTSUP,
    /// Value too large for defined data type
    EOVERFLOW,
    /// Operation not permitted
    EPERM,
    /// Broken pipe
    EPIPE,
    /// Protocol error
    EPROTO,
    /// Protocol not supported
    EPROTONOSUPPORT,
    /// Protocol wrong type for socket
    EPROTOTYPE,
    /// Result too large
    ERANGE,
    /// Read-only file system
    EROFS,
    /// Cannot send after transport endpoint shutdown
    ESHUTDOWN,
    /// Invalid seek
    ESPIPE,
    /// No such process
    ESRCH,
    /// Connection timed out
    ETIMEDOUT,
    /// Text file is busy
    ETXTBSY,
    /// Cross-device link not permitted
    EXDEV,
    /// Unknown error
    UNKNOWN,
    /// End of file
    EOF,
    /// No such device or address
    ENXIO,
    /// Too many links
    EMLINK,
    /// Inappropriate ioctl for device
    ENOTTY,
    /// Inappropriate file type or format
    EFTYPE,
    /// Illegal byte sequence
    EILSEQ,
    /// Socket type not supported
    ESOCKTNOSUPPORT,
};

/// If there is an error, create an appropriate error variant
///
/// Unfortunately, Zig's error sets don't support negative
/// values, and the default tag is a hardcoded `u16`
pub fn check(res: c_int) Error!void {
    if (res >= 0) return;
    return switch (res) {
        c.UV_E2BIG => Error.E2BIG,
        c.UV_EACCES => Error.EACCES,
        c.UV_EADDRINUSE => Error.EADDRINUSE,
        c.UV_EADDRNOTAVAIL => Error.EADDRNOTAVAIL,
        c.UV_EAFNOSUPPORT => Error.EAFNOSUPPORT,
        c.UV_EAGAIN => Error.EAGAIN,
        c.UV_EAI_ADDRFAMILY => Error.EAI_ADDRFAMILY,
        c.UV_EAI_AGAIN => Error.EAI_AGAIN,
        c.UV_EAI_BADFLAGS => Error.EAI_BADFLAGS,
        c.UV_EAI_BADHINTS => Error.EAI_BADHINTS,
        c.UV_EAI_CANCELED => Error.EAI_CANCELED,
        c.UV_EAI_FAIL => Error.EAI_FAIL,
        c.UV_EAI_FAMILY => Error.EAI_FAMILY,
        c.UV_EAI_MEMORY => Error.EAI_MEMORY,
        c.UV_EAI_NODATA => Error.EAI_NODATA,
        c.UV_EAI_NONAME => Error.EAI_NONAME,
        c.UV_EAI_OVERFLOW => Error.EAI_OVERFLOW,
        c.UV_EAI_PROTOCOL => Error.EAI_PROTOCOL,
        c.UV_EAI_SERVICE => Error.EAI_SERVICE,
        c.UV_EAI_SOCKTYPE => Error.EAI_SOCKTYPE,
        c.UV_EALREADY => Error.EALREADY,
        c.UV_EBADF => Error.EBADF,
        c.UV_EBUSY => Error.EBUSY,
        c.UV_ECANCELED => Error.ECANCELED,
        c.UV_ECHARSET => Error.ECHARSET,
        c.UV_ECONNABORTED => Error.ECONNABORTED,
        c.UV_ECONNREFUSED => Error.ECONNREFUSED,
        c.UV_ECONNRESET => Error.ECONNRESET,
        c.UV_EDESTADDRREQ => Error.EDESTADDRREQ,
        c.UV_EEXIST => Error.EEXIST,
        c.UV_EFAULT => Error.EFAULT,
        c.UV_EFBIG => Error.EFBIG,
        c.UV_EHOSTUNREACH => Error.EHOSTUNREACH,
        c.UV_EINTR => Error.EINTR,
        c.UV_EINVAL => Error.EINVAL,
        c.UV_EIO => Error.EIO,
        c.UV_EISCONN => Error.EISCONN,
        c.UV_EISDIR => Error.EISDIR,
        c.UV_ELOOP => Error.ELOOP,
        c.UV_EMFILE => Error.EMFILE,
        c.UV_EMSGSIZE => Error.EMSGSIZE,
        c.UV_ENAMETOOLONG => Error.ENAMETOOLONG,
        c.UV_ENETDOWN => Error.ENETDOWN,
        c.UV_ENETUNREACH => Error.ENETUNREACH,
        c.UV_ENFILE => Error.ENFILE,
        c.UV_ENOBUFS => Error.ENOBUFS,
        c.UV_ENODEV => Error.ENODEV,
        c.UV_ENOENT => Error.ENOENT,
        c.UV_ENOMEM => Error.ENOMEM,
        c.UV_ENONET => Error.ENONET,
        c.UV_ENOPROTOOPT => Error.ENOPROTOOPT,
        c.UV_ENOSPC => Error.ENOSPC,
        c.UV_ENOSYS => Error.ENOSYS,
        c.UV_ENOTCONN => Error.ENOTCONN,
        c.UV_ENOTDIR => Error.ENOTDIR,
        c.UV_ENOTEMPTY => Error.ENOTEMPTY,
        c.UV_ENOTSOCK => Error.ENOTSOCK,
        c.UV_ENOTSUP => Error.ENOTSUP,
        c.UV_EOVERFLOW => Error.EOVERFLOW,
        c.UV_EPERM => Error.EPERM,
        c.UV_EPIPE => Error.EPIPE,
        c.UV_EPROTO => Error.EPROTO,
        c.UV_EPROTONOSUPPORT => Error.EPROTONOSUPPORT,
        c.UV_EPROTOTYPE => Error.EPROTOTYPE,
        c.UV_ERANGE => Error.ERANGE,
        c.UV_EROFS => Error.EROFS,
        c.UV_ESHUTDOWN => Error.ESHUTDOWN,
        c.UV_ESPIPE => Error.ESPIPE,
        c.UV_ESRCH => Error.ESRCH,
        c.UV_ETIMEDOUT => Error.ETIMEDOUT,
        c.UV_ETXTBSY => Error.ETXTBSY,
        c.UV_EXDEV => Error.EXDEV,
        c.UV_UNKNOWN => Error.UNKNOWN,
        c.UV_EOF => Error.EOF,
        c.UV_ENXIO => Error.ENXIO,
        c.UV_EMLINK => Error.EMLINK,
        c.UV_ENOTTY => Error.ENOTTY,
        c.UV_EFTYPE => Error.EFTYPE,
        c.UV_EILSEQ => Error.EILSEQ,
        c.UV_ESOCKTNOSUPPORT => Error.ESOCKTNOSUPPORT,
        else => unreachable,
    };
}

/// Mode used to run the loop with
pub const RunMode = enum(c_uint) {
    DEFAULT = c.UV_RUN_DEFAULT,
    ONCE = c.UV_RUN_ONCE,
    NOWAIT = c.UV_RUN_NOWAIT,
};

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
    try loop.run(RunMode.DEFAULT);
    // Close the loop
    try loop.close();
}
