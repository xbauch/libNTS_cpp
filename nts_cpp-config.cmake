add_library(NTS_cpp SHARED IMPORTED)
find_library(NTS_cpp_LIBRARY_PATH NTS_cpp HINTS "${CMAKE_CURRENT_LIST_DIR}/../../")
set_target_properties(NTS_cpp PROPERTIES IMPORTED_LOCATION "${NTS_cpp_LIBRARY_PATH}")
