cmake_minimum_required(VERSION 2.8)
project(libNTS_cpp)

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

list ( APPEND CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra")

add_library ( NTS_cpp SHARED
	"data_types.cpp"
	"nts.cpp"
	"logic.cpp"
	"sugar.cpp"
)

add_executable ( ntslib_test
	"test.cpp"
)

target_link_libraries ( ntslib_test NTS_cpp )

install (
	FILES
		"nts.hpp"
		"data_types.hpp"
		"logic.hpp"
		"sugar.hpp"
	DESTINATION
		"include"
)

install (
	TARGETS
		"NTS_cpp"
	DESTINATION
		"lib"
	EXPORT
		"NTS_cpp-targets"
)

install (
	EXPORT
		"NTS_cpp-targets"
	DESTINATION
		"lib/cmake/NTS_cpp"
)

install (
	FILES
		"nts_cpp-config.cmake"
	DESTINATION
		"lib/cmake/NTS_cpp"
)
