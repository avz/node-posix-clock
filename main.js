var binding = require(__dirname + '/build/Release/posix-clock');

/**
 * The functions retrieve the time of the specified clock clockId.
 * @param {Integer} clockId Clock identifier, specified by constant
 * @returns {Object} Struct {sec: 12233, nsec: 3433434}
 */
exports.gettime = binding.gettime;

/**
 * The function finds the resolution (precision) of the
 * specified clock clockId and stores it in the struct timespec.
 * The resolution of clocks depends on the implementation and cannot be
 * configured by a particular process.
 * @param {Integer} clockId Clock identifier, specified by constant
 * @returns {Object} Struct {sec: 12233, nsec: 3433434}
 */
exports.getres = binding.getres;

/**
 * System-wide clock that measures real (i.e., wall-clock) time.
 * Setting this clock requires appropriate privileges.
 * This clock is affected by discontinuous jumps in
 * the system time (e.g., if the system administrator
 * manually changes the clock), and by the incremental adjustments
 * performed by adjtime(3) and NTP.
 */
exports.REALTIME = binding.REALTIME;

/**
 * Clock that cannot be set and represents monotonic time since some
 * unspecified starting point. This clock is not affected by discontinuous
 * jumps in the system time (e.g., if the system administrator
 * manually changes the clock), but is affected by the incremental adjustments
 * performed by adjtime(3) and NTP.
 */
exports.MONOTONIC = binding.MONOTONIC;

/**
 * @warning since Linux 2.6.32; Linux-specific
 * A faster but less precise version of CLOCK_REALTIME.
 * Use when you need very fast, but not fine-grained timestamps.
 */
exports.REALTIME_COARSE = binding.REALTIME_COARSE;

/**
 * @warning since Linux 2.6.32; Linux-specific
 * A faster but less precise version of CLOCK_MONOTONIC.
 * Use when you need very fast, but not fine-grained timestamps.
 */
exports.MONOTONIC_COARSE = binding.MONOTONIC_COARSE;

/**
 * @warning since Linux 2.6.28; Linux-specific
 * Similar to CLOCK_MONOTONIC, but provides access to a raw hardware-based time
 * that is not subject to NTP adjustments or the incremental adjustments
 * performed by adjtime(3).
 */
exports.MONOTONIC_RAW = binding.MONOTONIC_RAW;

/**
 * @warning since Linux 2.6.39; Linux-specific
 * Identical to CLOCK_MONOTONIC, except it also includes
 * any time that the system is suspended.  This allows applications to get
 * a suspend-aware monotonic clock without having to deal with
 * the complications of CLOCK_REALTIME, which may have discontinuities
 * if the time is changed using settimeofday(2).
 */
exports.BOOTTIME = binding.BOOTTIME;

/**
 * @warning since Linux 2.6.12
 * High-resolution per-process timer from the CPU.
 */
exports.PROCESS_CPUTIME_ID = binding.PROCESS_CPUTIME_ID;

/**
 * @warning since Linux 2.6.12
 * Thread-specific CPU-time clock.
 */
exports.THREAD_CPUTIME_ID = binding.THREAD_CPUTIME_ID;
