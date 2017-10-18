cmake_minimum_required(VERSION 2.8)

project(rtspace)

set(CMAKE_VERBOSE_MAKEFILE off)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Platform specified settings
IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    Message("Current Platform -- Linux")

    # Shared lib suffix
    set(dll_ext ".so")

    # OpenCL
    find_package(OpenCL)
    if(OPENCL_FOUND)
        INCLUDE_DIRECTORIES(${OpenCL_INCLUDE_DIRS} /usr/local/include)
        LINK_DIRECTORIES(${OpenCL_LIBRARY})
    else()
        #OpenCL cuda7.5 version
        set(OpenCL_DIR /usr/local/cuda-7.5)
        set(OpenCL_LIBRARIES OpenCL)
        INCLUDE_DIRECTORIES(${OpenCL_DIR}/include)
        LINK_DIRECTORIES(${OpenCL_DIR}/lib64)
    endif()

ELSE(MATCHES "Darwin")

    Message("Current Platform -- Darwin")

    # Shared lib suffix
    set(dll_ext ".dylib")

    # Set osx rpath
    set( CMAKE_MACOSX_RPATH 1)
    set( MACOSX_RPATH "@executable_path")

    #OpenCL
    find_package(OpenCL REQUIRED)
    INCLUDE_DIRECTORIES(${OpenCL_INCLUDE_DIRS} /usr/local/include)
    LINK_DIRECTORIES(${OpenCL_LIBRARY})

ELSE(MATCHES "Windows")

    Message("Current Platform -- Windows")

    #shared lib suffix
    set(dll_ext ".dll")
ENDIF()

# Extra clean files
list(APPEND clean.extra_files "")
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${clean_extra_files}")


IF( ${CMAKE_CXX_COMPILER} MATCHES  "[cg][+][+]")
    MESSAGE("G++ ${CMAKE_CXX_COMPILER}")
    set(CMAKE_CXX_FLAGS_DEBUG "-g3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_C_FLAGS_DEBUG "-g3")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS "-Wall  -std=c++11 -march=native -mtune=native")
    set(CMAKE_C_FLAGS   "-Wall  -std=gnu99 -march=native -mtune=native")

ELSEIF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    MESSAGE("${CMAKE_CXX_COMPILER} can try -DCMAKE_C_COMPILER=gcc-6 -DCMAKE_CXX_COMPILER=g++-6")
    set(CMAKE_CXX_FLAGS "-Wall  -std=c++11 -march=native -mtune=native")
    set(CMAKE_C_FLAGS   "-Wall  -std=gnu99 -march=native -mtune=native")


ELSE()
    MESSAGE("Unspecified compiler")
ENDIF()

# OpenMP
if(USE_OPENMP)
    FIND_PACKAGE( OpenMP)
    if(OPENMP_FOUND)
        message("OPENMP FOUND")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    endif()

ENDIF(USE_OPENMP)

# Libraries

# Jansson lib
FIND_LIBRARY(jansson_libs NAMES jansson
                PATHS /usr/local /usr
            )
FIND_PATH(jansson_header jansson.h)
INCLUDE_DIRECTORIES(${jansson_header})

##LIB EAL
FIND_PATH(eal_header lmice_trace.h
    PATHS eal)
LIST(APPEND eal_SRCS
    eal/lmice_trace.c
    eal/lmice_eal_shm.c
    eal/lmice_eal_hash.c
    eal/lmice_eal_spinlock.c
    eal/lmice_eal_malloc.c
    eal/lmice_eal_event.c
    eal/lmice_eal_time.c
    eal/lmice_eal_aio.c
    eal/lmice_eal_thread.c
    eal/lmice_eal_common.h
    )


# EAL Static Library
ADD_LIBRARY(eal_static STATIC ${eal_SRCS})

SET_TARGET_PROPERTIES(eal_static PROPERTIES OUTPUT_NAME "eal")
SET_TARGET_PROPERTIES(eal_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(eal_static PROPERTIES VERSION 1.2 SOVERSION 1)

INCLUDE_DIRECTORIES(${eal_header})

# Document
LIST(APPEND doc_files LICENSE
    README.md
    )
add_custom_target(Document ALL SOURCES ${doc_files})
