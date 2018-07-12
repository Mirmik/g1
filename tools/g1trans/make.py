#!/usr/bin/env python3
#coding: utf-8

import licant
from licant.cxx_modules import application
from licant.libs import include
import os

licant.libs.include("g1")
licant.libs.include("gxx")

application("g1trans", 
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

@licant.routine
def install():
	licant.do("g1trans")
	os.system("cp g1trans /usr/local/bin")

licant.ex("g1trans")