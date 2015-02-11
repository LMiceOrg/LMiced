#coding: utf-8


from cffi import FFI
ffi = FFI()
ffi.cdef("""

int printf(const char * format, ...);
""")
C = ffi.dlopen(None)
arg = ffi.new("char[]", "秦12344皇")
C.printf(" hello %s!\n",arg)

