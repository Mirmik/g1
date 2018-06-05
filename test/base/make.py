#!/usr/bin/env python3
#coding: utf-8

import licant
import licant.libs
from licant.cxx_modules import application

licant.libs.include("gxx")

application("target",
	sources = ["main.cpp"],
	include_paths = ["../.."],
	include_modules = [
		("gxx", "posix"),
		("gxx.print", "cout"),
		("gxx.dprint", "cout"),
	]
)

licant.ex("target")