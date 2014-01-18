# POSIX `clock_*()` for NodeJS [![Build Status](https://secure.travis-ci.org/avz/node-posix-clock.png?branch=master)](http://travis-ci.org/avz/node-posix-clock)

## Installation
```
npm install posix-clock
```

## Examples
```javascript
var clock = require('posix-clock');

var clockResolution = clock.getres(clock.MONOTONIC);
console.log(
	'Resolution of CLOCK_MONOTONIC: '
		+ clockResolution.sec + ' sec and '
		+ clockResolution.nsec + ' nanosec.'
	, clockResolution
);

var clockTime = clock.gettime(clock.MONOTONIC);
console.log(
	'Time from CLOCK_MONOTONIC: '
		+ clockTime.sec + ' sec and '
		+ clockTime.nsec + ' nanosec.'
	, clockTime
);

```

Result
```
% node test.js
Resolution of CLOCK_MONOTONIC: 0 sec and 1 nanosec. { sec: 0, nsec: 1 }
Time from CLOCK_MONOTONIC: 15224 sec and 557776233 nanosec. { sec: 15224, nsec: 557776233 }
```

## API

### Methods

 * `gettime(clockId)` - the function retrieve the time from the specified clock clockId.
 * `getres(clockId)` - the function return the resolution (precision) of the
specified clock clockId. The resolution of clocks depends on the implementation and cannot be
configured by a particular process.

### Clocks

 * `REALTIME` - system-wide clock that measures real (i.e., wall-clock) time.
Setting this clock requires appropriate privileges.
This clock is affected by discontinuous jumps in
the system time (e.g., if the system administrator
manually changes the clock), and by the incremental adjustments
performed by adjtime(3) and NTP.

 * `MONOTONIC` - clock that cannot be set and represents monotonic time since some
unspecified starting point. This clock is not affected by discontinuous
jumps in the system time (e.g., if the system administrator
manually changes the clock), but is affected by the incremental adjustments
performed by adjtime(3) and NTP.

 * `PROCESS_CPUTIME_ID` - *since Linux 2.6.12*.
High-resolution per-process timer from the CPU.

 * `THREAD_CPUTIME_ID` - *since Linux 2.6.12*.
Thread-specific CPU-time clock.

 * `REALTIME_COARSE` - *since Linux 2.6.32; Linux-specific*.
A faster but less precise version of `REALTIME`.
Use when you need very fast, but not fine-grained timestamps.

 * `MONOTONIC_COARSE` - *since Linux 2.6.32; Linux-specific*.
A faster but less precise version of `MONOTONIC`.
Use when you need very fast, but not fine-grained timestamps.

 * `MONOTONIC_RAW` - *since Linux 2.6.28; Linux-specific*.
Similar to `MONOTONIC`, but provides access to a raw hardware-based time
that is not subject to NTP adjustments or the incremental adjustments
performed by adjtime(3).

 * `BOOTTIME` - *since Linux 2.6.39; Linux-specific*
Identical to `MONOTONIC`, except it also includes
any time that the system is suspended.  This allows applications to get
a suspend-aware monotonic clock without having to deal with
the complications of `REALTIME`, which may have discontinuities
if the time is changed using settimeofday(2).

### See Also

See [man 2 clock_gettime](http://man7.org/linux/man-pages/man2/clock_gettime.2.html) for more details.
