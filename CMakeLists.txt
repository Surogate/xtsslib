cmake_minimum_required (VERSION 3.2)

project (xtsslib)

# the version number
set (XTSSLIB_VERSION_MAJOR 0)
set (XTSSLIB_VERSION_MINOR 1)

include ("utility.cmake")

find_boost_lib()

include_directories("${Boost_INCLUDE_DIRS}")

# directories where to search headers
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/test")
include_directories("${PROJECT_SOURCE_DIR}/fixed-container/include")

if (WIN32)
add_definitions(/std:c++latest)
add_definitions(/DCATCH_CPP14_OR_GREATER)
else (WIN32)
add_definitions(-std=c++17)
add_definitions(-DCATCH_CPP14_OR_GREATER)
endif(WIN32)

# sources in the resolver_server directory
set (XTSSLIB_SOURCES 
	${PROJECT_SOURCE_DIR}/test/main.cpp
	${PROJECT_SOURCE_DIR}/test/test_matrix.cpp
	${PROJECT_SOURCE_DIR}/test/test_string_view.cpp
	${PROJECT_SOURCE_DIR}/test/test_uri.cpp
	${PROJECT_SOURCE_DIR}/test/test_vector_deck.cpp
	
	${PROJECT_SOURCE_DIR}/aarray_view.hpp
	${PROJECT_SOURCE_DIR}/afilesystem.hpp
	${PROJECT_SOURCE_DIR}/afilesystem_fwd.hpp
	${PROJECT_SOURCE_DIR}/astring_view.hpp
	${PROJECT_SOURCE_DIR}/astring_view_fwd.hpp
	${PROJECT_SOURCE_DIR}/compatibility.hpp
	${PROJECT_SOURCE_DIR}/fast_convert.hpp
	${PROJECT_SOURCE_DIR}/file_operation.hpp
	${PROJECT_SOURCE_DIR}/matrix.hpp
	${PROJECT_SOURCE_DIR}/operation_type.hpp
	${PROJECT_SOURCE_DIR}/ParserLL.hpp
	${PROJECT_SOURCE_DIR}/return_status.hpp
	${PROJECT_SOURCE_DIR}/test.hpp
	${PROJECT_SOURCE_DIR}/test_uri.hpp
	${PROJECT_SOURCE_DIR}/Tree.hpp
	${PROJECT_SOURCE_DIR}/trim.hpp
	${PROJECT_SOURCE_DIR}/uri.hpp
	${PROJECT_SOURCE_DIR}/uri_builder.hpp
	${PROJECT_SOURCE_DIR}/vector_deck.hpp
	${PROJECT_SOURCE_DIR}/interprocess/linux_named_recursive_mutex.hpp
	${PROJECT_SOURCE_DIR}/interprocess/named_recursive_mutex.hpp
	${PROJECT_SOURCE_DIR}/interprocess/null_named_recursive_mutex.hpp
	${PROJECT_SOURCE_DIR}/interprocess/windows_named_recursive_mutex.hpp
	)

add_executable(xtsslib_test ${XTSSLIB_SOURCES})
