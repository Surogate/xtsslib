cmake_minimum_required (VERSION 3.7)

project (xtsslib)

#compiler requirement
set (CMAKE_CXX_STANDARD 14)

# the version number
set (RESOLVER_VERSION_MAJOR 0)
set (RESOLVER_VERSION_MINOR 1)

set(BOOST_ROOT_DIR ${PROJECT_SOURCE_DIR} CACHE STRING "The root directory of boost, leave it as is if you initialized subrepositories")

if (${BOOST_ROOT_DIR} EQUAL ${PROJECT_SOURCE_DIR})
	include_directories("${PROJECT_SOURCE_DIR}/utility/include")
	include_directories("${PROJECT_SOURCE_DIR}/config/include")
	include_directories("${PROJECT_SOURCE_DIR}/throw_exception/include")
	include_directories("${PROJECT_SOURCE_DIR}/assert/include")
else (${BOOST_ROOT_DIR} EQUAL ${PROJECT_SOURCE_DIR})
	include_directories("${BOOST_ROOT_DIR}")
endif(${BOOST_ROOT_DIR} EQUAL ${PROJECT_SOURCE_DIR})

# directories where to search headers
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/../Catch/include")

# sources in the resolver_server directory
set (XTSSLIB_SOURCES 
	${PROJECT_SOURCE_DIR}/test/main.cpp
	${PROJECT_SOURCE_DIR}/test/test_string_view.cpp
	${PROJECT_SOURCE_DIR}/test/test_uri.cpp
	${PROJECT_SOURCE_DIR}/aarray_view.hpp
	${PROJECT_SOURCE_DIR}/afilesystem.hpp
	${PROJECT_SOURCE_DIR}/afilesystem_fwd.hpp
	${PROJECT_SOURCE_DIR}/astring_view.hpp
	${PROJECT_SOURCE_DIR}/astring_view_fwd.hpp
	${PROJECT_SOURCE_DIR}/InputParsing.hpp
	${PROJECT_SOURCE_DIR}/ParserLL.hpp
	${PROJECT_SOURCE_DIR}/return_status.hpp
	${PROJECT_SOURCE_DIR}/test.hpp
	${PROJECT_SOURCE_DIR}/test_uri.hpp
	${PROJECT_SOURCE_DIR}/Tree.hpp
	${PROJECT_SOURCE_DIR}/trim.hpp
	${PROJECT_SOURCE_DIR}/uri.hpp
	${PROJECT_SOURCE_DIR}/uri_builder.hpp
	${PROJECT_SOURCE_DIR}/interprocess/linux_named_recursive_mutex.hpp
	${PROJECT_SOURCE_DIR}/interprocess/named_recursive_mutex.hpp
	${PROJECT_SOURCE_DIR}/interprocess/null_named_recursive_mutex.hpp
	${PROJECT_SOURCE_DIR}/interprocess/windows_named_recursive_mutex.hpp
	)

add_executable(xtsslib_test ${XTSSLIB_SOURCES})
