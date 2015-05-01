
add_executable ( ntslib_test
	"test.cpp"
)
target_include_directories ( ntslib_test PRIVATE "../src" )

target_link_libraries ( ntslib_test NTS_cpp )

add_executable ( inliner_test
	"test_inliner.cpp"
)
target_include_directories ( inliner_test PRIVATE "../src" )
target_link_libraries ( inliner_test NTS_cpp )

