# This CMakeLists was inspired by
# https://github.com/forexample/package-example/blob/master/Foo/CMakeLists.txt

set ( lib_name "libnts_cpp" )

add_library ( NTS_cpp SHARED
	"data_types.cpp"
	"variables.cpp"
	"nts.cpp"
	"logic.cpp"
	"sugar.cpp"
	"inliner.cpp"
)
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${lib_name}-configVersion.cmake")
set(project_config "${generated_dir}/${lib_name}-config.cmake")
set(targets_export_name "${lib_name}-targets")
#set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file(
	"${version_config}"
	COMPATIBILITY AnyNewerVersion
)

configure_file("Config.cmake.in" "${project_config}" @ONLY)

install(
	TARGETS NTS_cpp
	EXPORT "${targets_export_name}"
	LIBRARY DESTINATION "lib"
	ARCHIVE DESTINATION "lib"
	RUNTIME DESTINATION "bin"
	INCLUDES DESTINATION "${include_install_dir}"
)

# Install all header files
install(
	FILES
		"nts.hpp"
		"variables.hpp"
		"logic.hpp"
		"sugar.hpp"
		"data_types.hpp"
		"inliner.hpp"

	DESTINATION
		"${include_install_dir}/libNTS"
)

# Configuration files
install(
	FILES
		"${project_config}"
		"${version_config}"
	DESTINATION
		"${config_install_dir}"
)

install(
	EXPORT
		"${targets_export_name}"
#	NAMESPACE
#		"${namespace}"
	DESTINATION
		"${config_install_dir}"
)
