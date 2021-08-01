cmake_minimum_required(VERSION 3.18)

set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

include(cmake/DiscoverTests.cmake)

project(mop
        LANGUAGES CXX
        )
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options("-g3")
add_compile_options("$<$<CONFIG:DEBUG>:-Og>")

add_compile_options("$<$<CONFIG:RELEASE>:-Os>")
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

set(CMAKE_CXX_VISIBILITY_PRESET "hidden")

enable_testing()

find_program(PATH_CCACHE NAMES ccache)
if (PATH_CCACHE)
    # set(CMAKE_CXX_COMPILER_LAUNCHER "${PATH_CCACHE}")
endif ()

set(LINT ON)
if (LINT)
    add_compile_options("-Werror")
    add_compile_options(
            "-Wall"
            "-Wextra"
            "-pedantic"
    )
    add_compile_options(
            "$<$<CXX_COMPILER_ID:GNU>:-Wno-unknown-pragmas>"
            "$<$<CXX_COMPILER_ID:Clang>:-Wno-gnu-zero-variadic-macro-arguments>"
            "$<$<CXX_COMPILER_ID:Clang>:-Wno-unknown-warning-option>"
    )

    set(CMAKE_LINK_WHAT_YOU_USE OFF)

    add_link_options(
            "LINKER:-z,defs"
            "LINKER:--no-undefined"
            "LINKER:--no-allow-shlib-undefined"
    )

    find_program(PATH_CLANG_TIDY NAMES clang-tidy)
    if (PATH_CLANG_TIDY)
        set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
        set(CLANG_TIDY_CXXFLAGS)
        find_program(FOUND_NIX NAMES nix-build)
        if (FOUND_NIX)
            execute_process(
                    COMMAND bash -c "cat $(dirname $(which clang))/../nix-support/{cc-c,libc-c,libcxx-cxx}flags"
                    OUTPUT_VARIABLE output
            )
            string(REPLACE "\n" " " output "${output}")
            string(REGEX REPLACE " +" ";" output "${output}")
            foreach (arg ${output})
                list(APPEND CLANG_TIDY_CXXFLAGS "--extra-arg=${arg}")
            endforeach ()
        endif ()
        set(CMAKE_CXX_CLANG_TIDY "${PATH_CLANG_TIDY};-p=${CMAKE_CURRENT_BINARY_DIR};${CLANG_TIDY_CXXFLAGS}")
    endif ()

    find_program(PATH_CPPCHECK NAMES cppcheck)
    if (PATH_CPPCHECK)
        set(CMAKE_CXX_CPPCHECK "${PATH_CPPCHECK};--suppress=preprocessorErrorDirective")
    endif ()

    find_program(PATH_CPPLINT NAMES cpplint)
    if (PATH_CPPLINT)
        set(CMAKE_CXX_CPPLINT "${PATH_CPPLINT};--filter=-legal/copyright,-readability/namespace,-runtime/indentation_namespace,-whitespace")
    endif ()

    find_program(PATH_IWYU NAMES include-what-you-use)
    if (PATH_IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${PATH_IWYU}")
    endif ()
endif ()

# main

add_executable(mop
        modules/main/src/main.cpp
        )
target_link_libraries(mop INTERFACE tier2)

# libtest

add_library(libtest)
target_sources(libtest
        PRIVATE
        modules/test/test.cpp
        )

# liballoc

add_library(liballoc)
target_sources(liballoc
        PRIVATE
        modules/alloc/alloc.cpp
        )

# libtier0

add_library(libtier0 INTERFACE)
target_sources(libtier0
        INTERFACE
        modules/tier0/tier0.hpp
        )

add_executable(libtier0-tests)
target_link_libraries(libtier0-tests PRIVATE libtest liballoc INTERFACE libtier0)
target_discover_tests(libtier0-tests)
target_sources(libtier0-tests
        PRIVATE
        modules/tier0/tests.cpp
        )

# libtier1

add_library(libtier1 INTERFACE)
target_link_libraries(libtier1 INTERFACE libtier0)
target_sources(libtier1
        INTERFACE
        modules/tier1/tier1.hpp
        )

add_executable(libtier1-tests)
target_link_libraries(libtier1-tests PRIVATE libtest liballoc INTERFACE libtier1)
target_discover_tests(libtier1-tests)
target_sources(libtier1-tests
        PRIVATE
        modules/tier1/tests.cpp
        )

# libtier2

add_library(libtier2 INTERFACE)
target_link_libraries(libtier2 INTERFACE libtier1)
target_link_libraries(libtier2
        INTERFACE
        modules/tier2/tier2.hpp
        )

add_executable(libtier2-tests)
target_link_libraries(libtier2-tests PRIVATE libtest liballoc INTERFACE libtier2)
target_discover_tests(libtier2-tests)
target_sources(libtier2-tests
        PRIVATE
        modules/tier2/tests.cpp
        )
