#!/usr/bin/env python
# -*- coding: utf-8 -*-

from global_defines import *
import os
import subprocess
from cffi import FFI

ffi = FFI()

shm_source ="#include \""    \
    + os.path.abspath(os.path.dirname( __file__ )  \
    + "/../src/lmice_eal_shm.c") +"\""

trace_source ="#include \""    \
    + os.path.abspath(os.path.dirname( __file__ )  \
    + "/../src/lmice_trace.c") +"\""

shm_header = os.path.abspath(os.path.dirname( __file__ )  \
    + "/../src/lmice_eal_shm.h")

ffi.cdef("""
struct lmice_shm_s
{
    int fd;
    int size;
    uint64_t addr;
    char name[32];
};

void *
     memcpy(void *restrict dst, const void *restrict src, size_t n);

typedef struct lmice_shm_s lmice_shm_t;

int eal_shm_create(lmice_shm_t* shm);

int eal_shm_destroy(lmice_shm_t* shm);

int eal_shm_open(lmice_shm_t* shm, int mode);

int eal_shm_openex(lmice_shm_t* shm, int mode, int size, int offset);

int eal_shm_close(lmice_shm_t* shm);

void eal_shm_zero(lmice_shm_t* shm);
"""
)
#
#subprocess.Popen([
#'gcc', '-E', '-Dinline=__inline', '-D_DEBUG=1',
#shm_header], stdout=subprocess.PIPE).communicate()[0]
#

print trace_source

C = ffi.verify(trace_source + '\n' + shm_source , \
    define_macros=g_define_macros, \
    extra_compile_args=g_extra_compile_args, \
    libraries= g_libraries) # or a list of libraries

global shm

def shmstep0():
    global shm
    shm = ffi.new("lmice_shm_t *")
    C.eal_shm_zero(shm)
    shm.name = "hello"*4
    shm.size = 4096*4096*10
    print ffi.string(shm.name), shm.fd, shm.size, hex(shm.addr)

def shmstep1x():
    global shm
    ret = 0
    ret = C.eal_shm_openex(shm, 2, 4097, 4096)
    print "ret errno", ret, ffi.errno
    print "shm.addr is ", hex(shm.addr)
    if ret != 0:
        print "errno is ", ffi.errno
        assert ret == 0

def shmstep1():
    global shm

    ret = 0
    ret = C.eal_shm_create(shm)
    print "ret errno", ret, ffi.errno
    if ret != 0:
        print "errno is ", ffi.errno
        assert ret == 0
    ret = C.eal_shm_open(shm, 2)
    print "ret errno", ret, ffi.errno
    print "shm.addr is ", hex(shm.addr)
    if ret != 0:
        print "errno is ", ffi.errno
        assert ret == 0
def shmstep2():
    ret = C.eal_shm_close(shm)
    print "ret errno", ret, ffi.errno
    if ret != 0:
        print "errno is ", ffi.errno
        assert ret == 0
    ret = C.eal_shm_destroy(shm)
    print "ret errno", ret, ffi.errno
    if ret != 0:
        print "errno is ", ffi.errno
        assert ret == 0
def test_shm():
    shmstep0()
    shmstep1()
    shmstep2()
def setup_function(function):
    print ("setting up %s" % function)

if __name__ == "__main__":
    global shm
    import sys
    shmstep0()
    print shm.fd, shm.size,ffi.string(shm.name)
    c= sys.stdin.readline()
    print "readlins is return",c
    if c[:4] == "open":
        print "into openex test process 2"
        shmstep1x()
        print "copy 7 bytes "
        caddr = ffi.cast("void*", shm.addr)
        C.memcpy(caddr, "welcome", 7)
        sys.stdin.readline()
        print "copy %d bytes" % (4096*2+1)
        C.memcpy(caddr, "welcome"*2000, 4096*2+1)
        sys.stdin.readline()
        shmstep2()
    elif c[:5] == "close":
        pass
        #C.shm_unlink(shm.name)
    else:
        shmstep1()
        print "into open test process 1"
        sys.stdin.readline()
        shmstep2()
