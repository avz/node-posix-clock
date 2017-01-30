{
	"targets": [
		{
			"target_name": "posix-clock",
			"sources": ["src/posix-clock.cpp"],
			"include_dirs" : [
				"<!(node -e \"require('nan')\")"
			]
		}
	]
}
