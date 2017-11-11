cmake_minimum_required(VERSION 2.8)
project(silis)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SOURCE_FILES
        src/lib/macro.h
        src/lib/number.c
        src/lib/vector.c
        src/lib/buffer.c
        src/lib/string.c

        src/ctx.c
        src/main.c

        src/phases/parse.c
        src/phases/flatten.c
        src/phases/print.c
        src/phases/eval.c

        src/intrinsics/debug/puti.c
        src/intrinsics/debug/puts.c

        src/intrinsics/_.h
        src/intrinsics/cond.c
        src/intrinsics/define.c
        src/intrinsics/do.c
        src/intrinsics/func.c

        src/intrinsics/minus.c
        src/intrinsics/plus.c
        )
add_executable(silis ${SOURCE_FILES})
target_link_libraries(silis m)
