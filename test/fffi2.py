#coding: utf-8

import cffi
ffi = cffi.FFI()
C = ffi.verify("""
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pwd.h>
#include <uuid/uuid.h>
     """, libraries=[])
ffi.cdef("""


     struct passwd {
                        char    *pw_name;       /* user name */
                        char    *pw_passwd;     /* encrypted password */
                        int   pw_uid;         /* user uid */
                        int   pw_gid;         /* user gid */
                        time_t  pw_change;      /* password change time */
                        char    *pw_class;      /* user access class */
                        char    *pw_gecos;      /* Honeywell login info */
                        char    *pw_dir;        /* home directory */
                        char    *pw_shell;      /* default shell */
                        time_t  pw_expire;      /* account expiration */
                        int     pw_fields;      /* internal: fields filled in */
                };

     struct passwd *
          getpwuid(int uid);

""")

p = ffi.getpwuid(0)
print ffi.string(p.pw_name)
