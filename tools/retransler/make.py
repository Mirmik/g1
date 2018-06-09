#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.libs
from licant.cxx_modules import application
from licant.modules import submodule
import licant.cxx_make

licant.libs.include("gxx")
licant.execute("../../g1.g.py")

application("g1-retransler",
	sources = ["main.cpp"],
	include_paths = ["../.."],
	include_modules = [
		("gxx", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.log2"),
		("gxx.inet", "posix"),

		("g1"),
		("g1.allocator", "malloc"),
	],

	#binutils = bu,
	libs = ["pthread", "readline"]
)

licant.make.copy(src="g1-retransler", tgt="/usr/local/bin/g1-retransler")
licant.make.add_makefile_target("install", ["/usr/local/bin/g1-retransler"])

licant.ex("g1-retransler")