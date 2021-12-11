cmake_minimum_required(VERSION 3.18)

set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

include(cmake/DiscoverTests.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(silis
        LANGUAGES CXX
        )
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>")

# OBJECT | STATIC | SHARED
set(BUILD_LIBS_MODE SHARED)

add_compile_options("$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-g3>")
add_compile_options("$<$<AND:$<CONFIG:DEBUG>,$<CXX_COMPILER_ID:AppleClang,Clang,GNU>>:-Og>")

add_compile_options("$<$<CONFIG:RELEASE>:-Os>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

set(CMAKE_CXX_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
add_compile_definitions(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

enable_testing()

find_program(PATH_CCACHE NAMES ccache)
if (PATH_CCACHE)
    # set(CMAKE_CXX_COMPILER_LAUNCHER "${PATH_CCACHE}")
endif ()

set(LINT ON)
set(LINT_CLANG_TIDY ON)
set(LINT_CPPCHECK ON)
set(LINT_CPPLINT ON)
set(LINT_IWYU ON)

if (WIN32)
    set(LINT_CLANG_TIDY OFF)
endif ()

if (LINT)
    add_compile_options(
            # warnings as errors
            "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Werror>"
            "$<$<CXX_COMPILER_ID:MSVC>:/WX>"

            # base warnings
            "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wall>"
            "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wextra>"
            "$<$<CXX_COMPILER_ID:AppleClang,Clang>:-Weverything>"
            "$<$<CXX_COMPILER_ID:MSVC>:/Wall>"
            "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-pedantic>"
            "$<$<CXX_COMPILER_ID:GNU>:-pedantic-errors>"
            "$<$<CXX_COMPILER_ID:MSVC>:/permissive->"

            # extra warnings
            "$<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wconsumed>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wcast-align=strict>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wmissing-declarations>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wredundant-decls>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wshadow>"
            "$<$<CXX_COMPILER_ID:GNU>:-fstrict-aliasing>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wstrict-aliasing=3>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wstrict-overflow=5>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wtrampolines>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wuninitialized>"
            "$<$<CXX_COMPILER_ID:GNU>:-Wunused>"

            # accepted warnings
            "$<$<CXX_COMPILER_ID:Clang>:-Wno-c++20-compat-pedantic>"
            "$<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wno-gnu-zero-variadic-macro-arguments>" # c++20-compat
            "$<$<CXX_COMPILER_ID:AppleClang,Clang>:-Wno-c++98-compat-pedantic>"
            "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-Wno-unknown-pragmas>"
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4068>" # unknown pragma
            "$<$<CXX_COMPILER_ID:MSVC>:/D_CRT_SECURE_NO_WARNINGS>"
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4291>" # operator new matching operator delete
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4371>" # layout may have changed due to better packing
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4464>" # relative include path contains '..'
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4514>" # unreferenced inline function
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4623>" # default constructor was implicitly defined as deleted
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4625>" # copy constructor was implicitly defined as deleted
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4626>" # assignment operator was implicitly defined as deleted
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4643>" # forward declaring in namespace std
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4702>" # unreachable code (occurs in recursive templates)
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4710>" # function not inlined
            "$<$<CXX_COMPILER_ID:MSVC>:/wd4711>" # selected for automatic inline expansion
            "$<$<CXX_COMPILER_ID:MSVC>:/wd5027>" # move assignment operator was implicitly defined as deleted
            "$<$<CXX_COMPILER_ID:MSVC>:/wd5030>" # unrecognized attribute
            "$<$<CXX_COMPILER_ID:MSVC>:/wd5045>" # spectre mitigation
            "$<$<CXX_COMPILER_ID:MSVC>:/wd5105>" # macro expansion producing 'defined'
    )

    set(CMAKE_LINK_WHAT_YOU_USE OFF)

    add_link_options(
            "$<$<PLATFORM_ID:Linux>:LINKER:-z,defs>"
            "$<$<PLATFORM_ID:Linux>:LINKER:--no-undefined>"
            "$<$<PLATFORM_ID:Linux>:LINKER:--no-allow-shlib-undefined>"
    )

    find_program(PATH_CLANG_TIDY NAMES clang-tidy)
    if (LINT_CLANG_TIDY AND PATH_CLANG_TIDY)
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
    if (LINT_CPPCHECK AND PATH_CPPCHECK)
        set(CMAKE_CXX_CPPCHECK "${PATH_CPPCHECK};--suppress=preprocessorErrorDirective")
    endif ()

    find_program(PATH_CPPLINT NAMES cpplint)
    if (LINT_CPPLINT AND PATH_CPPLINT)
        set(CMAKE_CXX_CPPLINT "${PATH_CPPLINT};--filter=-legal/copyright,-readability/namespace,-runtime/indentation_namespace,-whitespace")
    endif ()

    find_program(PATH_IWYU NAMES include-what-you-use)
    if (LINT_IWYU AND PATH_IWYU)
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${PATH_IWYU}")
    endif ()
endif ()

set(targets "")
macro(hook_target target)
    list(APPEND targets "${target}")
endmacro()

macro(add_library target)
    _add_library(${target} ${BUILD_LIBS_MODE} ${ARGN})
    hook_target(${target})
endmacro()

macro(add_executable target)
    _add_executable(${target} ${ARGN})
    hook_target(${target})
endmacro()

macro(add_custom_target target)
    _add_custom_target(${target} ${ARGN})
    hook_target(${target})
endmacro()

cmake_language(DEFER CALL hook_targets_complete)
function(hook_targets_complete)
    foreach (target ${targets})
        configure_target(${target})
    endforeach ()
endfunction()

function(configure_target target)
    get_target_property(target_type ${target} TYPE)
    message(STATUS "defined target ${target_type}: ${target}")
    if (target_type STREQUAL "UTILITY")
        return()
    endif ()

    set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)

    get_property(x_host TARGET ${target} PROPERTY X_HOST)
    if (NOT x_host)
        target_compile_options(${target} PRIVATE
                "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-ffreestanding>"
                "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-nostdlib>"
                "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-nostdinc>"
                )
    endif ()

    set(BUILD_MACRO "${target}_exports")
    string(REGEX REPLACE "[^0-9A-Z_a-z]" "_" BUILD_MACRO "${BUILD_MACRO}")
    string(TOUPPER "${BUILD_MACRO}" BUILD_MACRO)
    set_target_properties(${target}
            PROPERTIES
            DEFINE_SYMBOL "${BUILD_MACRO}"
            )
    # target_compile_definitions(${target} PRIVATE "${BUILD_MACRO}")

    if (BUILD_LIBS_MODE STREQUAL "OBJECT")
        target_sources(${target} INTERFACE $<TARGET_OBJECTS:${target}>)
    endif ()

    if (target MATCHES "-tests$")
        target_link_libraries(${target} PRIVATE libtest)
    endif ()
endfunction()

# libtest

add_library(libtest)
set_target_properties(libtest PROPERTIES X_HOST ON)
target_sources(libtest PRIVATE
        modules/test/test.hpp modules/test/test.cpp
        )

# liballoc

add_library(liballoc)
set_target_properties(liballoc PROPERTIES X_HOST ON)
target_link_libraries(liballoc PUBLIC libtier0)
target_sources(liballoc PRIVATE
        modules/alloc/alloc.hpp modules/alloc/alloc.cpp
        )

# libtier0

add_library(libtier0)
target_sources(libtier0 PRIVATE
        modules/tier0/tier0.hpp modules/tier0/tier0.cpp
        )

add_executable(libtier0-tests)
target_link_libraries(libtier0-tests PUBLIC libtier0)
target_discover_tests(libtier0-tests TEST_SOURCE_DIR modules/tier0)
target_sources(libtier0-tests PRIVATE
        modules/tier0/tests.cpp
        )

# libtier1

add_library(libtier1)
target_link_libraries(libtier1 PUBLIC libtier0)
target_link_libraries(libtier1 PUBLIC liballoc)
target_sources(libtier1 PRIVATE
        modules/tier1/tier1.hpp modules/tier1/tier1.cpp
        )

add_executable(libtier1-tests)
target_link_libraries(libtier1-tests PUBLIC libtier1)
target_discover_tests(libtier1-tests TEST_SOURCE_DIR modules/tier1)
target_sources(libtier1-tests PRIVATE
        modules/tier1/tests.cpp
        )

# libtier2

add_library(libtier2)
target_link_libraries(libtier2 PUBLIC libtier1)
target_sources(libtier2 PRIVATE
        modules/tier2/tier2.hpp modules/tier2/tier2.cpp
        )

add_executable(libtier2-tests)
target_link_libraries(libtier2-tests PUBLIC libtier2)
target_discover_tests(libtier2-tests TEST_SOURCE_DIR modules/tier2)
target_sources(libtier2-tests PRIVATE
        modules/tier2/tests.cpp
        )

# kernel

add_library(libkernel)
target_link_libraries(libkernel PUBLIC libtier1)
target_sources(libkernel PRIVATE
        modules/kernel/kernel.hpp modules/kernel/kernel.cpp
        )

# kernel linux impl

add_library(libkernel-linux)
set_target_properties(libkernel-linux PROPERTIES X_HOST ON)
target_link_libraries(libkernel-linux PUBLIC libtier1)
target_sources(libkernel-linux PRIVATE
        modules/kernel-linux/kernel-linux.cpp
        )

# libnet

add_library(libnet)
set_target_properties(libnet PROPERTIES X_HOST ON)
target_link_libraries(libnet PUBLIC libtier0)
target_sources(libnet PRIVATE
        modules/net/net.hpp modules/net/net.cpp
        )

# test-libnet

add_executable(test-libnet)
set_target_properties(test-libnet PROPERTIES X_HOST ON)
target_link_libraries(test-libnet PUBLIC libnet)
target_link_libraries(test-libnet PUBLIC libkernel-linux)
target_sources(test-libnet PRIVATE
        modules/test-net/main.cpp
        )

# scriptengine-jvm

add_library(scriptengine-jvm)
target_link_libraries(scriptengine-jvm PUBLIC libtier2)
target_sources(scriptengine-jvm PRIVATE
        modules/scriptengine-jvm/classfile.hpp
        modules/scriptengine-jvm/classfile-constants.inc
        modules/scriptengine-jvm/decode.hpp
        modules/scriptengine-jvm/descriptor.hpp
        modules/scriptengine-jvm/eval.cpp
        modules/scriptengine-jvm/instructions.inc modules/scriptengine-jvm/instructions.hpp
        modules/scriptengine-jvm/loadclass.cpp
        modules/scriptengine-jvm/loadcode.cpp
        modules/scriptengine-jvm/scriptengine.hpp modules/scriptengine-jvm/scriptengine.cpp
        )

add_executable(scriptengine-jvm-tests)
install(TARGETS scriptengine-jvm-tests DESTINATION .)
target_link_libraries(scriptengine-jvm-tests PUBLIC scriptengine-jvm)
target_link_libraries(scriptengine-jvm-tests PUBLIC libkernel-linux)
target_discover_tests(scriptengine-jvm-tests TEST_SOURCE_DIR modules/scriptengine-jvm/tests)
target_sources(scriptengine-jvm-tests PRIVATE
        modules/scriptengine-jvm/tests/tests.cpp
        )

find_package(Java 11 REQUIRED)
function(target_java_source target srcfile)
    string(REGEX REPLACE "[.]java$" ".class" dstfile ${srcfile})
    string(REGEX REPLACE "[^A-Za-z0-9_.+-]" "_" classtarget ${dstfile})
    set(classtarget "java_${classtarget}")
    set(dstfile "${CMAKE_CURRENT_BINARY_DIR}/${dstfile}")
    get_filename_component(dstdir "${dstfile}" DIRECTORY)
    set(srcfile "${CMAKE_CURRENT_SOURCE_DIR}/${srcfile}")
    get_filename_component(srcdir "${srcfile}" DIRECTORY)
    add_custom_target(${classtarget}
            DEPENDS "${dstfile}"
            )
    add_custom_command(
            VERBATIM
            OUTPUT "${dstfile}"
            DEPENDS "${srcfile}"
            COMMAND "${Java_JAVAC_EXECUTABLE}" -cp "${srcdir}" -d "${dstdir}" -implicit:none "${srcfile}"
    )
    add_dependencies("${target}" "${classtarget}")
endfunction()
target_java_source(scriptengine-jvm-tests modules/scriptengine-jvm/scripts/ExtractJRT.java)

set(extractJRTDir "${CMAKE_CURRENT_BINARY_DIR}/modules/scriptengine-jvm/scripts")
set(extractJRTClass "ExtractJRT")
set(JRT "${CMAKE_CURRENT_BINARY_DIR}/modules/scriptengine-jvm/rt")
add_custom_target(rt
        DEPENDS "${JRT}"
        )
add_custom_command(
        VERBATIM
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules/scriptengine-jvm"
        OUTPUT "${JRT}.jar"
        DEPENDS "${extractJRTDir}/${extractJRTClass}.class"
        COMMAND "${Java_JAVA_EXECUTABLE}" -cp "${extractJRTDir}" "${extractJRTClass}"
        COMMAND "mkdir" "rt"
)
add_custom_command(
        VERBATIM
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/modules/scriptengine-jvm/rt"
        OUTPUT "${JRT}"
        DEPENDS "${JRT}.jar"
        COMMAND "${CMAKE_COMMAND}" -E tar xf "${JRT}.jar"
)
add_dependencies(scriptengine-jvm-tests rt)

target_java_source(scriptengine-jvm-tests modules/scriptengine-jvm/tests/Hello.java)
