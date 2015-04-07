cmake_minimum_required(VERSION 2.8)
project(libNTS_cpp)

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

list( APPEND CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -Wextra")


add_library ( NTS_cpp STATIC
	"data_types.cpp"
	"nts.cpp"
	"./logic.cpp"
)

add_executable ( ntslib_test
	"test.cpp"
)
target_link_libraries ( ntslib_test NTS_cpp )
