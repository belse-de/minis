cmake_minimum_required(VERSION 2.6)
project(mini_sh)

add_executable(mini_sh main.cpp)

set(CMAKE_C_FLAGS   ${CMAKE_C_FLAGS}   "-std=c11 -O0 -g3 -Wall -Wextra -Wpedantic -fsanitize=address")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++14 -O0 -g3 -Wall -Wextra -Wpedantic -fsanitize=address")
set(CMAKE_LD_FLAGS ${CMAKE_LD_FLAGS} "-fsanitize=address")

install(TARGETS mini_sh RUNTIME DESTINATION bin)
