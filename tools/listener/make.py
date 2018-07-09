#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include

licant.libs.include("g1")
licant.libs.include("gxx")

application("g1listener", 
	sources = ["main.cpp"],
	include_modules = [
		("g1"),
		("g1.allocator", "malloc"),
		("g1.time", "chrono"),
		
		("gxx", "posix"),
		("gxx.log2", "impl"),
		("gxx.inet", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
		("gxx.syslock", "mutex"),
		("gxx.serial"),
	],
	cxx_flags = "",
	libs = ["pthread"]
)

licant.ex("g1listener")