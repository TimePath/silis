cmake_minimum_required(VERSION 2.8)
project(silis C)

set(CMAKE_C_STANDARD 11)

include(CheckCCompilerFlag)
function(try_flag LIST FLAG)
    string(REGEX REPLACE "[/=-]" "_" TEST ${FLAG})
    set(CMAKE_REQUIRED_FLAGS "-Werror")
    check_c_compiler_flag("${FLAG}" ${TEST})
    set(CMAKE_REQUIRED_FLAGS "")
    if (${TEST})
        set(${LIST} "${${LIST}} ${FLAG}" PARENT_SCOPE)
    endif ()
endfunction()

try_flag(CMAKE_C_FLAGS "-Werror")
try_flag(CMAKE_C_FLAGS "-pedantic")
try_flag(CMAKE_C_FLAGS "-Wall")
try_flag(CMAKE_C_FLAGS "-Wextra")
try_flag(CMAKE_C_FLAGS "-Weverything")

# keep
try_flag(CMAKE_C_FLAGS "-Wno-keyword-macro")
try_flag(CMAKE_C_FLAGS "-Wno-reserved-id-macro")
try_flag(CMAKE_C_FLAGS "-Wno-unused-command-line-argument")
try_flag(CMAKE_C_FLAGS "-Wno-unused-macros")

include_directories(src)

set(BUILD_SHARED_LIBS OFF)

set(TEST ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)

function(main)
    add_library(silis-runtime
            src/system.h

            src/lib/encoding/ascii.c
            src/lib/allocator.c
            src/lib/buffer.c
            src/lib/fs.c
            src/lib/macro.c
            src/lib/slice.c
            src/lib/stdio.c
            src/lib/string.c
            src/lib/trie.c
            src/lib/vector.c
            )

    add_library(silis-interpreter
            src/compiler/node.c
            src/compiler/symbol.c
            src/compiler/symbols.c
            src/compiler/token.c
            src/compiler/type.c
            src/compiler/types.c
            src/compiler/value.c

            src/compiler/phases/01-lex/lex.c
            src/compiler/phases/02-parse/parse.c
            src/compiler/phases/03-eval/eval.c
            )
    target_link_libraries(silis-interpreter silis-runtime)

    add_library(silis-compiler
            src/compiler/compilation.c
            src/compiler/env.c

            src/compiler/phases/04-emit/emit.c

            src/compiler/targets/_.c
            src/compiler/targets/c.c
            src/compiler/targets/js.c

            src/compiler/intrinsics/_.c
            src/compiler/intrinsics/debug/puti.c
            src/compiler/intrinsics/debug/puts.c
            src/compiler/intrinsics/cond.c
            src/compiler/intrinsics/define.c
            src/compiler/intrinsics/do.c
            src/compiler/intrinsics/emit.c
            src/compiler/intrinsics/extern.c
            src/compiler/intrinsics/func.c
            src/compiler/intrinsics/if.c
            src/compiler/intrinsics/include.c
            src/compiler/intrinsics/minus.c
            src/compiler/intrinsics/plus.c
            src/compiler/intrinsics/set.c
            src/compiler/intrinsics/untyped.c
            src/compiler/intrinsics/while.c

            src/compiler/intrinsics/types/func.c
            )
    target_link_libraries(silis-compiler silis-interpreter)

    add_executable(silisc src/compiler/main.c)
    target_link_libraries(silisc silis-compiler)

    add_executable(silisc-test src/test.c)

    silisc_test(tests/01-simple.sil)
    silisc_test(tests/02-extern.sil)
    silisc_test(tests/03-if.sil)
    silisc_test(tests/04-while.sil)
endfunction()

enable_testing()
set(silisc_tests "")
function(add_tests)
    if (NOT ${TEST})
        return()
    endif ()
    add_custom_target(
            tests
            ALL
            DEPENDS ${CMAKE_BINARY_DIR}/tested
    )
    add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/tested
            DEPENDS silisc-test silisc ${silisc_tests}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failure
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/tested
    )
endfunction()

set(ENV ${CMAKE_COMMAND} -E env)
if (${CMAKE_VERSION} VERSION_LESS "3.1.0")
    set(ENV env)
endif ()
function(silisc_test file)
    set(pwd ${CMAKE_CURRENT_SOURCE_DIR})
    get_filename_component(name ${file} NAME)
    set(name test-${name})
    add_test(
            NAME ${name}
            COMMAND ${ENV}
            CC=${CMAKE_C_COMPILER}
            $<TARGET_FILE:silisc-test> $<TARGET_FILE:silisc> ${pwd} ${pwd} ${file}
    )
    list(APPEND silisc_tests ${pwd}/${file})
    set(silisc_tests ${silisc_tests} PARENT_SCOPE)
endfunction()

if (TinyCC)
    set(CMAKE_C_ARCHIVE_CREATE "tcc -ar cr <TARGET> <LINK_FLAGS> <OBJECTS>")
endif ()

if (EMSCRIPTEN)
    set(TEST OFF)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s RETAIN_COMPILER_SETTINGS=1 -s NODERAWFS=1")

    set(EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES ON)
    set(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
    set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_C_COMPILER> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_CREATE_STATIC_LIBRARY "<CMAKE_CXX_COMPILER> -o <TARGET> <LINK_FLAGS> <OBJECTS>")
endif ()

main()

add_tests()
