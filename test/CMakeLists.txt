add_executable ( ntslib_test
	"test.cpp"
)

include_directories ( "../src" )
target_link_libraries ( ntslib_test NTS_cpp )

