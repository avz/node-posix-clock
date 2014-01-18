var clock = require('../');

var requiredClockTypes = [
	'REALTIME',
	'MONOTONIC'
];

var optionalClockTypes = [
	'REALTIME_COARSE',
	'MONOTONIC_COARSE',
	'MONOTONIC_RAW',
	'BOOTTIME',
	'PROCESS_CPUTIME_ID',
	'THREAD_CPUTIME_ID'
];

exports.gettime = function(test) {
	requiredClockTypes.forEach(function(clockId) {
		var result = clock.gettime(clock[clockId]);

		test.notEqual(result.sec, undefined);
		test.notEqual(result.nsec, undefined);
	});

	optionalClockTypes.forEach(function(clockId) {
		if(!clock[clockId]) {
			// clock is not supported by system
			return;
		}

		var result = clock.gettime(clock[clockId]);

		test.notEqual(result.sec, undefined);
		test.notEqual(result.nsec, undefined);
	});

	test.done();
}

exports.getres = function(test) {
	requiredClockTypes.forEach(function(clockId) {
		var result = clock.getres(clock[clockId]);

		test.notEqual(result.sec, undefined);
		test.notEqual(result.nsec, undefined);
	});

	optionalClockTypes.forEach(function(clockId) {
		if(!clock[clockId]) {
			// clock is not supported by system
			return;
		}

		var result = clock.getres(clock[clockId]);

		test.notEqual(result.sec, undefined);
		test.notEqual(result.nsec, undefined);
	});

	test.done();
}
