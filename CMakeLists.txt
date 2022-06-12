#
# Libevent CMake project
#
# Based on initial work by:
#    Alexey Ozeritsky
#
# Additional changes:
#   Brodie Thiesfield
#   Joakim Soderberg
#   Trond Norbye
#   Sergei Nikulov
#
#   Build example:
#
#       cd libevent
#       md build
#       cd build
#       cmake -G "Visual Studio 10" ..
#       start libevent.sln
#

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()
if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()
if (POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Set build type to Debug o Release (default Release)" FORCE)
endif()

# get rid of the extra default configurations
# what? why would you get id of other useful build types? - Ellzey
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Limited configurations" FORCE)

project(libevent C)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

# equals to VERSION_INFO in Makefile.am
set(EVENT_ABI_LIBVERSION_CURRENT   7)
set(EVENT_ABI_LIBVERSION_REVISION  1)
set(EVENT_ABI_LIBVERSION_AGE       0)

# equals to RELEASE in Makefile.am
set(EVENT_PACKAGE_RELEASE 2.1)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
#
# But only if this variables are not defined yet
# (i.e. libevent is used via add_subdirectory())
if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()
if (NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

set(GNUC 0)
if (("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU"))
    set(GNUC 1)
endif()

set(EVENT_LIBRARY_STATIC ON)
set(EVENT_LIBRARY_SHARED ON)

# GNUC specific options.
option(EVENT__DISABLE_GCC_WARNINGS "Disable verbose warnings with GCC" OFF)
option(EVENT__ENABLE_GCC_HARDENING "Enable compiler security checks" OFF)
option(EVENT__ENABLE_GCC_FUNCTION_SECTIONS "Enable gcc function sections" OFF)
option(EVENT__ENABLE_GCC_WARNINGS "Make all GCC warnings into errors" OFF)

set(GCC_V ${CMAKE_C_COMPILER_VERSION})

list(APPEND __FLAGS
        -Wall -Wextra -Wno-unused-parameter -Wstrict-aliasing -Wstrict-prototypes

        -fno-strict-aliasing # gcc 2.9.5+
        -Wmissing-prototypes

        # gcc 4
        -Winit-self
        -Wmissing-field-initializers
        -Wdeclaration-after-statement

        # gcc 4.2
        -Waddress
        -Wnormalized=id
        -Woverride-init

        # gcc 4.5
        -Wlogical-op

        -Wwrite-strings
)

if (EVENT__DISABLE_GCC_WARNINGS)
    list(APPEND __FLAGS -w)
endif()

if (EVENT__ENABLE_GCC_HARDENING)
    list(APPEND __FLAGS
            -fstack-protector-all
            -fwrapv
            -fPIE
            -Wstack-protector
            "--param ssp-buffer-size=1")

    add_definitions(-D_FORTIFY_SOURCE=2)
endif()

if (EVENT__ENABLE_GCC_FUNCTION_SECTIONS)
    list(APPEND __FLAGS -ffunction-sections)
    # TODO: Add --gc-sections support. We need some checks for NetBSD to ensure this works.
endif()

if (EVENT__ENABLE_GCC_WARNINGS)
    list(APPEND __FLAGS -Werror)
endif()

macro(add_compiler_flags)
	foreach(flag ${ARGN})
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flag}")
	endforeach()
endmacro()

add_compiler_flags(${__FLAGS})

add_definitions(-D_GNU_SOURCE=1)
set(CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)

# Group the source files.
set(HDR_PRIVATE
    bufferevent-internal.h
    changelist-internal.h
    defer-internal.h
    epolltable-internal.h
    evbuffer-internal.h
    event-internal.h
    evmap-internal.h
    evrpc-internal.h
    evsignal-internal.h
    evthread-internal.h
    ht-internal.h
    http-internal.h
    log-internal.h
    minheap-internal.h
    mm-internal.h
    ratelim-internal.h
    strlcpy-internal.h
    util-internal.h
    evconfig-private.h
    compat/sys/queue.h)

set(HDR_COMPAT
    include/evdns.h
    include/evrpc.h
    include/event.h
    include/evhttp.h
    include/evutil.h)

set(HDR_PUBLIC
    include/event2/buffer.h
    include/event2/bufferevent.h
    include/event2/bufferevent_compat.h
    include/event2/bufferevent_struct.h
    include/event2/buffer_compat.h
    include/event2/dns.h
    include/event2/dns_compat.h
    include/event2/dns_struct.h
    include/event2/event.h
    include/event2/event_compat.h
    include/event2/event_struct.h
    include/event2/http.h
    include/event2/http_compat.h
    include/event2/http_struct.h
    include/event2/keyvalq_struct.h
    include/event2/listener.h
    include/event2/rpc.h
    include/event2/rpc_compat.h
    include/event2/rpc_struct.h
    include/event2/tag.h
    include/event2/tag_compat.h
    include/event2/thread.h
    include/event2/util.h
    include/event2/visibility.h
    ${PROJECT_BINARY_DIR}/include/event2/event-config.h)

set(SRC_CORE
    buffer.c
    bufferevent.c
    bufferevent_filter.c
    bufferevent_pair.c
    bufferevent_ratelim.c
    bufferevent_sock.c
    event.c
    evmap.c
    evthread.c
    evutil.c
    evutil_rand.c
    evutil_time.c
    listener.c
    log.c
    signal.c
    strlcpy.c)

list(APPEND SRC_CORE epoll.c)

if (NOT EVENT__DISABLE_THREAD_SUPPORT)
    find_package(Threads REQUIRED)
    if (NOT CMAKE_USE_PTHREADS_INIT)
        message(FATAL_ERROR
                "Failed to find Pthreads, set EVENT__DISABLE_THREAD_SUPPORT to disable")
    endif()

    set(EVENT__HAVE_PTHREADS 1)
    list(APPEND LIB_APPS ${CMAKE_THREAD_LIBS_INIT})
endif()

set(SRC_EXTRA
    event_tagging.c
    http.c
    evdns.c
    evrpc.c)

add_definitions(-DHAVE_CONFIG_H)

# We use BEFORE here so we don't accidentally look in system directories
# first for some previous versions of the headers that are installed.
include_directories(BEFORE ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/compat
    ${PROJECT_SOURCE_DIR}/include)

set(EVENT_SHARED_FLAGS -fvisibility=hidden)

#
# Create the libraries.
#
macro(set_event_shared_lib_flags LIB_NAME)
    set_target_properties("${LIB_NAME}_shared" PROPERTIES
        COMPILE_FLAGS ${ARGN})
    set_target_properties("${LIB_NAME}_shared" PROPERTIES
        LINK_FLAGS ${ARGN})
endmacro()
macro(generate_pkgconfig LIB_NAME)
    set(prefix      ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix ${CMAKE_INSTALL_PREFIX})
    set(libdir      ${CMAKE_INSTALL_PREFIX}/lib)
    set(includedir  ${CMAKE_INSTALL_PREFIX}/include)

    set(VERSION ${EVENT_ABI_LIBVERSION})

    set(LIBS         "")
    foreach (LIB ${LIB_PLATFORM})
        set(LIBS "${LIBS} -L${LIB}")
    endforeach()
endmacro()
macro(add_event_library LIB_NAME)
    cmake_parse_arguments(LIB
        "" # Options
        "VERSION" # One val
        "SOURCES;LIBRARIES;INNER_LIBRARIES;OUTER_INCLUDES" # Multi val
        ${ARGN}
    )

    if ("${LIB_OUTER_INCLUDES}" STREQUAL "")
        set(LIB_OUTER_INCLUDES NONE)
    endif()
    set(ADD_EVENT_LIBRARY_INTERFACE)

    if (${EVENT_LIBRARY_STATIC})
        add_library("${LIB_NAME}_static" STATIC ${LIB_SOURCES})
        set_target_properties("${LIB_NAME}_static" PROPERTIES
            OUTPUT_NAME "${LIB_NAME}"
            CLEAN_DIRECT_OUTPUT 1)

        if(LIB_INNER_LIBRARIES)
            set(INNER_LIBRARIES "${LIB_INNER_LIBRARIES}_static")
        endif()
        target_link_libraries("${LIB_NAME}_static"
            ${CMAKE_THREAD_LIBS_INIT}
            ${LIB_PLATFORM}
            ${INNER_LIBRARIES}
            ${LIB_LIBRARIES})

        set(ADD_EVENT_LIBRARY_INTERFACE "${LIB_NAME}_static")
    endif()

    if (${EVENT_LIBRARY_SHARED})
        add_library("${LIB_NAME}_shared" SHARED ${LIB_SOURCES})

        if(LIB_INNER_LIBRARIES)
            set(INNER_LIBRARIES "${LIB_INNER_LIBRARIES}_shared")
        endif()
        target_link_libraries("${LIB_NAME}_shared"
            ${CMAKE_THREAD_LIBS_INIT}
            ${LIB_PLATFORM}
            ${INNER_LIBRARIES}
            ${LIB_LIBRARIES})

        if (EVENT_SHARED_FLAGS)
            set_event_shared_lib_flags("${LIB_NAME}" "${EVENT_SHARED_FLAGS}")
        endif()

        math(EXPR CURRENT_MINUS_AGE "${EVENT_ABI_LIBVERSION_CURRENT}-${EVENT_ABI_LIBVERSION_AGE}")
        set_target_properties(
            "${LIB_NAME}_shared" PROPERTIES
            OUTPUT_NAME "${LIB_NAME}-${EVENT_PACKAGE_RELEASE}"
            VERSION "${CURRENT_MINUS_AGE}.${EVENT_ABI_LIBVERSION_AGE}.${EVENT_ABI_LIBVERSION_REVISION}"
            SOVERSION "${CURRENT_MINUS_AGE}"
            INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

        set(LIB_LINK_NAME
            "${CMAKE_SHARED_LIBRARY_PREFIX}${LIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")

        add_custom_command(TARGET ${LIB_NAME}_shared
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                "$<TARGET_FILE_NAME:${LIB_NAME}_shared>"
                "${LIB_LINK_NAME}"
            WORKING_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

        set(ADD_EVENT_LIBRARY_INTERFACE "${LIB_NAME}_shared")
    endif()

    add_library(${LIB_NAME} INTERFACE)
    target_link_libraries(${LIB_NAME} INTERFACE ${ADD_EVENT_LIBRARY_INTERFACE})

    generate_pkgconfig("${LIB_NAME}")
endmacro()

add_event_library(event SOURCES ${SRC_CORE} ${SRC_EXTRA})

message(STATUS "")
message(STATUS "        ---( Libevent )---")
message(STATUS "")
message(STATUS "Available event backends: ${BACKENDS}")
message(STATUS "CMAKE_BINARY_DIR:         ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR:         ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR:       ${PROJECT_BINARY_DIR}")
message(STATUS "PROJECT_SOURCE_DIR:       ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_MODULE_PATH:        ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_COMMAND:            ${CMAKE_COMMAND}")
message(STATUS "CMAKE_ROOT:               ${CMAKE_ROOT}")
message(STATUS "CMAKE_SYSTEM:             ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_NAME:        ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_VERSION:     ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR:   ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SKIP_RPATH:         ${CMAKE_SKIP_RPATH}")
message(STATUS "CMAKE_VERBOSE_MAKEFILE:   ${CMAKE_VERBOSE_MAKEFILE}")
message(STATUS "CMAKE_C_FLAGS:            ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_BUILD_TYPE:         ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER:         ${CMAKE_C_COMPILER} (id ${CMAKE_C_COMPILER_ID}, clang ${CLANG}, GNUC ${GNUC})")
message(STATUS "CMAKE_AR:                 ${CMAKE_AR}")
message(STATUS "CMAKE_RANLIB:             ${CMAKE_RANLIB}")
message(STATUS "")

