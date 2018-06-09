#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.libs
from licant.cxx_modules import application
from licant.modules import submodule

licant.libs.include("gxx")
licant.execute("../../g1.g.py")

application("target",
	sources = ["main.cpp"],
	include_paths = ["../.."],
	include_modules = [
		("gxx", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.log2"),

		("g1"),
		("g1.allocator", "malloc"),
	],

	libs = ["pthread", "readline"]
)

licant.ex("target")