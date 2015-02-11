#!/usr/bin/env python
# -*- coding: utf-8 -*-

from global_defines import *
import os
from cffi import FFI
ffi = FFI()
endian_source ="#include \""    \
    + os.path.abspath(os.path.dirname( __file__ )  \
    + "/../src/lmice_eal_endian.c") +"\""

ffi.cdef("""
inline int
eal_is_little_endian();

inline int
eal_is_big_endian();

""")

C = ffi.verify(endian_source, \
    define_macros=g_define_macros, \
    extra_compile_args=g_extra_compile_args, \
    libraries= g_libraries) # or a list of libraries

def test_endian():

    print "is_little_endian\t",  C.eal_is_little_endian()
    print "is_big_endian\t\t", C.eal_is_big_endian()
    assert C.eal_is_little_endian()+C.eal_is_big_endian() == 1

def setup_function(function):
    print ("setting up %s" % function)
