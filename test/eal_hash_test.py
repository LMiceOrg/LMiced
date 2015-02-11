#!/usr/bin/env python
# -*- coding: utf-8 -*-

from global_defines import *
import os
from cffi import FFI

ffi = FFI()
hash_source ="#include \""    \
    + os.path.abspath(os.path.dirname( __file__ )  \
    + "/../src/lmice_eal_hash.c") +"\""

ffi.cdef("""
uint64_t eal_hash64_fnv1a(const void* data, uint32_t size);
uint32_t eal_hash32_fnv1a(const void* data, uint32_t size);
""")

C = ffi.verify(hash_source, \
    define_macros=g_define_macros, \
    extra_compile_args=g_extra_compile_args, \
    libraries= g_libraries) # or a list of libraries

def test_hash():
    assert C.eal_hash32_fnv1a("hello", 5) == 1335831723
    assert C.eal_hash64_fnv1a("hello", 5) == 11831194018420276491

def setup_function(function):
    print ("setting up %s" % function)
