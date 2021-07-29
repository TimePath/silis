cmake_minimum_required(VERSION 3.18)
project(mop)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -O3")

add_executable(mop
        modules/tier0/tier0.hpp
        modules/tier1/tier1.hpp
        modules/tier2/tier2.hpp

        modules/main/src/host.cpp
        modules/main/src/main.cpp
        )
