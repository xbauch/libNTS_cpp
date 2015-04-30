cmake_minimum_required(VERSION 3.0)
project ( libNTS_cpp VERSION 0.1)


set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

list ( APPEND CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra")

add_subdirectory ( src )
add_subdirectory ( test )


