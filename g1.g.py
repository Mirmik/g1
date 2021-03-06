import licant.modules

licant.modules.module("g1",
	sources = [
		"g1/src/tower.cpp", "g1/src/packet.cpp",
	],
	include_paths=["."]
)

licant.modules.module("g1.allocator", "malloc",
	sources = [
		"g1/src/malloc_allocator.cpp"
	]
)

licant.modules.module("g1.udpgate",
	sources = [
		"g1/src/udpgate.cpp"
	]
)

licant.modules.module("g1.time", "chrono",
	sources = [
		"g1/src/stdtime.cpp"
	]
)