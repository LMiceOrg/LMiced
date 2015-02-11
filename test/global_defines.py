#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

g_define_macros=[('inline','__inline'),  ('_DEBUG', '1')]
g_extra_compile_args=[]
g_libraries=[]

g_os_name = os.uname()[0]
g_os_arch = os.uname()[4]
g_os_type = os.name

# check os name
if g_os_name == 'Darwin':
    print g_os_name
    g_extra_compile_args.append('-Wno-error=unused-command-line-argument-hard-error-in-future')
    g_extra_compile_args.append('-Wno-warning=unused-function')

#check os type
if g_os_type == 'posix':
    #ansi c standard
    g_extra_compile_args.append('-std=c89')
    #gcc unit-at-a-time optimization
    g_extra_compile_args.append('-funit-at-a-time')

#check os arch
if g_os_arch == 'x86_64':
    pass
