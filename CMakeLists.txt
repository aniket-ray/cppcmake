cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)

include(CheckSymbolExists)
include(CheckIPOSupported)

option(CPPCMAKE_BUILD_BINARY "Build cppcmake binary" ON)
option(CPPCMAKE_FORCE_PSELECT "Use pselect() even on platforms that provide ppoll()" OFF)

project(cppcmake CXX)

# --- optional link-time optimization
check_ipo_supported(RESULT lto_supported OUTPUT error)

if (lto_supported)
    message(STATUS "IPO / LTO enabled")
    set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
else ()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif ()


include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-Wno-deprecated flag_no_deprecated)
if (flag_no_deprecated)
    add_compile_options(-Wno-deprecated)
endif ()
if (CMAKE_VERSION VERSION_LESS 3.24)
    check_cxx_compiler_flag(-fdiagnostics-color flag_color_diag)
    if (flag_color_diag)
        add_compile_options(-fdiagnostics-color)
    endif ()
elseif (NOT DEFINED ENV{CMAKE_COLOR_DIAGNOSTICS})
    set(CMAKE_COLOR_DIAGNOSTICS ON)
endif ()

if (NOT CPPCMAKE_FORCE_PSELECT)
    # Check whether ppoll() is usable on the target platform.
    # Set -DUSE_PPOLL=1 if this is the case.
    #
    # NOTE: Use check_cxx_symbol_exists() instead of check_symbol_exists()
    # because on Linux, <poll.h> only exposes the symbol when _GNU_SOURCE
    # is defined.
    #
    # Both g++ and clang++ define the symbol by default, because the C++
    # standard library headers require it, but *not* gcc and clang, which
    # are used by check_symbol_exists().
    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(ppoll poll.h HAVE_PPOLL)
    if (HAVE_PPOLL)
        add_compile_definitions(USE_PPOLL=1)
    endif ()
endif ()


# --- optional re2c
set(RE2C_MAJOR_VERSION 0)
find_program(RE2C re2c)
if (RE2C)
    execute_process(COMMAND "${RE2C}" --vernum OUTPUT_VARIABLE RE2C_RAW_VERSION)
    math(EXPR RE2C_MAJOR_VERSION "${RE2C_RAW_VERSION} / 10000")
endif ()
if (${RE2C_MAJOR_VERSION} GREATER 1)
    # the depfile parser and ninja lexers are generated using re2c.
    function(re2c IN OUT)
        add_custom_command(DEPENDS ${IN} OUTPUT ${OUT}
                COMMAND ${RE2C} -b -i --no-generation-date --no-version -o ${OUT} ${IN}
        )
    endfunction()
    re2c(deps/depfile_parser.in.cc ${PROJECT_BINARY_DIR}/depfile_parser.cc)
    re2c(deps/lexer.in.cc ${PROJECT_BINARY_DIR}/lexer.cc)
    add_library(libninja-re2c OBJECT ${PROJECT_BINARY_DIR}/depfile_parser.cc ${PROJECT_BINARY_DIR}/lexer.cc)
else ()
    message(WARNING "re2c 2 or later was not found; changes to deps/*.in.cc will not affect your build.")
    add_library(libninja-re2c OBJECT deps/depfile_parser.cc deps/lexer.cc)
endif ()
target_include_directories(libninja-re2c PRIVATE deps)

# --- Check for 'browse' mode support
function(check_platform_supports_browse_mode RESULT)
    # Make sure the inline.sh script works on this platform.
    # It uses the shell commands such as 'od', which may not be available.

    execute_process(
            COMMAND sh -c "echo 'TEST' | deps/inline.sh var"
            WORKING_DIRECTORY .
            RESULT_VARIABLE inline_result
            OUTPUT_QUIET
            ERROR_QUIET
    )
    if (NOT inline_result EQUAL "0")
        # The inline script failed, so browse mode is not supported.
        set(${RESULT} "0" PARENT_SCOPE)
        if (NOT WIN32)
            message(WARNING "browse feature omitted due to inline script failure")
        endif ()
        return()
    endif ()

    # Now check availability of the unistd header
    check_symbol_exists(fork "unistd.h" HAVE_FORK)
    check_symbol_exists(pipe "unistd.h" HAVE_PIPE)
    set(browse_supported 0)
    if (HAVE_FORK AND HAVE_PIPE)
        set(browse_supported 1)
    endif ()
    set(${RESULT} "${browse_supported}" PARENT_SCOPE)
    if (NOT browse_supported)
        message(WARNING "browse feature omitted due to missing `fork` and `pipe` functions")
    endif ()

endfunction()

set(NINJA_PYTHON "python" CACHE STRING "Python interpreter to use for the browse tool")

check_platform_supports_browse_mode(platform_supports_ninja_browse)

# Core source files all build into ninja library.
add_library(libninja OBJECT
        deps/build_log.cc
        deps/build.cc
        deps/clean.cc
        deps/clparser.cc
        deps/dyndep.cc
        deps/dyndep_parser.cc
        deps/debug_flags.cc
        deps/deps_log.cc
        deps/disk_interface.cc
        deps/edit_distance.cc
        deps/eval_env.cc
        deps/graph.cc
        deps/graphviz.cc
        deps/json.cc
        deps/line_printer.cc
        deps/manifest_parser.cc
        deps/metrics.cc
        deps/missing_deps.cc
        deps/parser.cc
        deps/state.cc
        deps/status_printer.cc
        deps/string_piece_util.cc
        deps/util.cc
        deps/version.cc
)

target_sources(libninja PRIVATE deps/subprocess-posix.cc)
if (CMAKE_SYSTEM_NAME STREQUAL "OS400" OR CMAKE_SYSTEM_NAME STREQUAL "AIX")
    target_sources(libninja PRIVATE deps/getopt.c)
    # Build getopt.c, which can be compiled as either C or C++, as C++
    # so that build environments which lack a C compiler, but have a C++
    # compiler may build ninja.
    set_source_files_properties(deps/getopt.c PROPERTIES LANGUAGE CXX)
endif ()

# Needed for perfstat_cpu_total
if (CMAKE_SYSTEM_NAME STREQUAL "AIX")
    target_link_libraries(libninja PUBLIC "-lperfstat")
endif ()


target_compile_features(libninja PUBLIC cxx_std_11)


# On IBM i (identified as "OS400" for compatibility reasons) and AIX, this fixes missing
# PRId64 (and others) at compile time in C++ sources
if (CMAKE_SYSTEM_NAME STREQUAL "OS400" OR CMAKE_SYSTEM_NAME STREQUAL "AIX")
    add_compile_definitions(__STDC_FORMAT_MACROS)
endif ()

# Main executable is library plus main() function.
if (CPPCMAKE_BUILD_BINARY)
    add_executable(cppcmake src/cppcmake.cc
            src/cppcmake_utils.cpp
            src/cppcmake_utils.hpp
            src/cppcmake_backend.cpp
            src/cppcmake_backend.hpp)
    target_link_libraries(cppcmake PRIVATE libninja libninja-re2c)
endif ()

# Adds browse mode into the ninja binary if it's supported by the host platform.
if (platform_supports_ninja_browse)
    # Inlines deps/browse.py into the browse_py.h header, so that it can be included
    # by deps/browse.cc
    add_custom_command(
            OUTPUT build/browse_py.h
            MAIN_DEPENDENCY deps/browse.py
            DEPENDS deps/inline.sh
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/build
            COMMAND deps/inline.sh kBrowsePy
            < deps/browse.py
            > ${PROJECT_BINARY_DIR}/build/browse_py.h
            WORKING_DIRECTORY .
            VERBATIM
    )

    if (CPPCMAKE_BUILD_BINARY)
        target_compile_definitions(cppcmake PRIVATE NINJA_HAVE_BROWSE)
        target_sources(cppcmake PRIVATE deps/browse.cc)
    endif ()
    set_source_files_properties(deps/browse.cc
            PROPERTIES
            OBJECT_DEPENDS "${PROJECT_BINARY_DIR}/build/browse_py.h"
            INCLUDE_DIRECTORIES "${PROJECT_BINARY_DIR}"
            COMPILE_DEFINITIONS NINJA_PYTHON="${NINJA_PYTHON}"
    )
endif ()

include(CTest)
if (BUILD_TESTING)

    # Can be removed if cmake min version is >=3.24
    if (POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif ()

    find_package(GTest)
    if (NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
                googletest
                URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz
                URL_HASH SHA256=81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2
        )
        FetchContent_MakeAvailable(googletest)
    endif ()

    # Tests all build into cppcmake_test executable.
    add_executable(cppcmake_test
            deps/build_log_test.cc
            deps/build_test.cc
            deps/clean_test.cc
            deps/clparser_test.cc
            deps/depfile_parser_test.cc
            deps/deps_log_test.cc
            deps/disk_interface_test.cc
            deps/dyndep_parser_test.cc
            deps/edit_distance_test.cc
            deps/graph_test.cc
            deps/json_test.cc
            deps/lexer_test.cc
            deps/manifest_parser_test.cc
            deps/missing_deps_test.cc
            deps/cppcmake_test.cc
            deps/state_test.cc
            deps/string_piece_util_test.cc
            deps/subprocess_test.cc
            deps/test.cc
            deps/util_test.cc
    )

    find_package(Threads REQUIRED)
    target_link_libraries(cppcmake_test PRIVATE libninja libninja-re2c GTest::gtest Threads::Threads)


    if (CMAKE_SYSTEM_NAME STREQUAL "AIX" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        # These tests require more memory than will fit in the standard AIX shared stack/heap (256M)
        target_link_options(hash_collision_bench PRIVATE "-Wl,-bmaxdata:0x80000000")
        target_link_options(manifest_parser_perftest PRIVATE "-Wl,-bmaxdata:0x80000000")
    endif ()

    add_test(NAME NinjaTest COMMAND cppcmake_test)
endif ()

if (CPPCMAKE_BUILD_BINARY)
    install(TARGETS cppcmake)
endif ()

add_executable(cppcmake_unit_test unit_tests/test_make.cpp
        src/cppcmake_utils.cpp
        src/cppcmake_utils.hpp
        src/cppcmake_backend.cpp
        src/cppcmake_backend.hpp)
target_link_libraries(cppcmake_unit_test PRIVATE libninja libninja-re2c)

