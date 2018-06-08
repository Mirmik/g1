import licant.modules

licant.modules.module("g1",
	sources = [
		"g1/src/tower.cpp", "g1/src/packet.cpp", "g1/src/gateway.cpp" 
	]
)

licant.modules.module("g1.allocator", "malloc",
	sources = [
		"g1/src/malloc_allocator.cpp"
	]
)