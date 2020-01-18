cmake_minimum_required(VERSION 2.8)
project(silis C)

set(CMAKE_C_STANDARD 99)

include(CheckCCompilerFlag)
function(try_flag LIST FLAG)
    string(REGEX REPLACE "[/=-]" "_" TEST ${FLAG})
    if (WIN32)
        set(CMAKE_REQUIRED_FLAGS "")
    else ()
        set(CMAKE_REQUIRED_FLAGS "-Werror")
    endif ()
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
try_flag(CMAKE_C_FLAGS "/utf-8")

# keep
try_flag(CMAKE_C_FLAGS "-Wno-keyword-macro")
try_flag(CMAKE_C_FLAGS "-Wno-reserved-id-macro")
try_flag(CMAKE_C_FLAGS "-Wno-unreachable-code")
try_flag(CMAKE_C_FLAGS "-Wno-unused-command-line-argument")
try_flag(CMAKE_C_FLAGS "-Wno-unused-macros")
try_flag(CMAKE_C_FLAGS "/wd4204")  # non-constant aggregate initializer
try_flag(CMAKE_C_FLAGS "/wd4214")  # bit field types other than int
try_flag(CMAKE_C_FLAGS "/wd4464")  # relative include path contains '..'
try_flag(CMAKE_C_FLAGS "/wd5045")  # spectre mitigation

include_directories(src)
add_definitions(-DUSE_REAL_HEADERS=0)

set(BUILD_SHARED_LIBS OFF)

set(DEBUG OFF)
if (DEBUG)
    set(BUILD_SHARED_LIBS ON) # ensure libraries don't have undefined references
endif ()

set(TEST ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)

function(freestanding target)
    set(LINK_OPTIONS "")
    if (DEBUG)
        list(APPEND LINK_OPTIONS "-nodefaultlibs")
    endif ()
    if (NOT target STREQUAL "silis-runtime")
        list(APPEND LINK_OPTIONS "-Wl,--no-undefined") # check for undefined symbols
        # list(APPEND LINK_OPTIONS "-Wl,--no-allow-shlib-undefined") # also check dependencies
    endif ()
    set_target_properties(${target} PROPERTIES LINK_OPTIONS "${LINK_OPTIONS}")
endfunction()

function(main)
    add_library(silis-system
            src/prelude.c
            src/system.c
            )

    add_library(silis-runtime
            src/lib/encoding/ascii.c
            src/lib/fs/memoryfile.c
            src/lib/fs/native.c
            src/lib/allocator.c
            src/lib/buffer.c
            src/lib/env.c
            src/lib/fs.c
            src/lib/macro.c
            src/lib/misc.c
            src/lib/result.c
            src/lib/slice.c
            src/lib/stdio.c
            src/lib/string.c
            src/lib/trie.c
            src/lib/vector.c
            )
    freestanding(silis-runtime)

    add_library(silis-parser
            src/parser/error.c
            src/parser/lexer.c
            src/parser/node.c
            src/parser/parser.c
            src/parser/token.c
            )
    freestanding(silis-parser)
    target_link_libraries(silis-parser
            PRIVATE silis-runtime
            )

    add_library(silis-interpreter
            src/interpreter/eval.c
            src/interpreter/interpreter.c
            src/interpreter/intrinsic.c
            src/interpreter/nodelist.c
            src/interpreter/symbol.c
            src/interpreter/symbols.c
            src/interpreter/type.c
            src/interpreter/types.c
            src/interpreter/value.c
            )
    freestanding(silis-interpreter)
    target_link_libraries(silis-interpreter
            PRIVATE silis-runtime
            PUBLIC silis-parser
            )

    add_library(silis-compiler
            src/compiler/emit.c
            src/compiler/output.c

            src/compiler/targets/_.c
            src/compiler/targets/c.c
            src/compiler/targets/js.c

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
    freestanding(silis-compiler)
    target_link_libraries(silis-compiler
            PRIVATE silis-runtime
            PUBLIC silis-interpreter
            )

    add_library(exe-silisc src/compiler/main.c)
    freestanding(exe-silisc)
    target_link_libraries(exe-silisc
            PRIVATE silis-runtime
            PUBLIC silis-compiler
            )
    add_executable(silisc src/loader/main.c)
    target_link_libraries(silisc
            PRIVATE silis-system silis-runtime
            PUBLIC exe-silisc
            )

    add_executable(silisc-test src/test.c)

    silisc_test("01-simple.sil")
    silisc_test("02-extern.sil")
    silisc_test("03-if.sil")
    silisc_test("04-while.sil")
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
            COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1 ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/tested
    )
endfunction()

set(ENV ${CMAKE_COMMAND} -E env)
if (${CMAKE_VERSION} VERSION_LESS "3.1.0")
    set(ENV env)
endif ()
function(silisc_test file)
    set(pwd "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    get_filename_component(name ${file} NAME)
    set(name test-${name})
    set(label ${name})
    add_test(
            NAME translate-${name}
            WORKING_DIRECTORY ${pwd}
            COMMAND
            $<TARGET_FILE:silisc-test> translate $<TARGET_FILE:silisc> ${pwd} ${file}
    )
    set_property(TEST translate-${name} PROPERTY LABELS ${label})
    foreach (target c)
        add_test(
                NAME assemble-${target}-${name}
                WORKING_DIRECTORY ${pwd}
                COMMAND ${ENV}
                CC=${CMAKE_C_COMPILER}
                $<TARGET_FILE:silisc-test> assemble ${target} ${pwd} ${file}
        )
        set_property(TEST assemble-${target}-${name} PROPERTY LABELS ${label})
        set_property(TEST assemble-${target}-${name} APPEND PROPERTY DEPENDS translate-${name})

        add_test(
                NAME run-${target}-${name}
                WORKING_DIRECTORY ${pwd}
                COMMAND
                $<TARGET_FILE:silisc-test> run ${target} ${pwd} ${file}
        )
        set_property(TEST run-${target}-${name} PROPERTY LABELS ${label})
        set_property(TEST run-${target}-${name} APPEND PROPERTY DEPENDS assemble-${target}-${name})
    endforeach ()
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
