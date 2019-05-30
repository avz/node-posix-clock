{
	'targets': [
		{
			'target_name': 'posix-clock',
			'sources': ['src/posix-clock.cpp'],
			'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")"],
			'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
			'conditions': [
				['OS=="mac"', {
					'cflags+': ['-fvisibility=hidden'],
					'xcode_settings': {
					  'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
					}
				}]
			],
		 	'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
		}
	]
}
