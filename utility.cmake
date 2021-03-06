cmake_minimum_required (VERSION 3.2)

#input: a list of the boost library needed, can be empty
#output: Boost_LIBRARY_DIRS to be used with link_directories
#output: Boost_INCLUDE_DIRS to be used with include_directories
#output: Boost_LIBRARIES to be used with target_link_libraries
function(find_boost_lib)
	if (${UNIX})
		find_package(Boost REQUIRED COMPONENTS ${ARGN})
	else (${UNIX})
		set(BOOST_DIR ${PROJECT_SOURCE_DIR} CACHE PATH "The root directory of boost, leave it as is if you initialized subrepositories")
		set (BOOST_LIBRARY_DIR "${BOOST_DIR}/stage/lib" CACHE PATH "The directory where boost lib are generated")
		set(BOOST_ROOT "${BOOST_DIR}")
		set(BOOST_LIBRARYDIR "${BOOST_LIBRARY_DIR}")
		
		set(GEN_BOOST_STATIC_LIB TRUE CACHE BOOL "You have generated static boost lib")
		if (${GEN_BOOST_STATIC_LIB})
			set(Boost_USE_STATIC_LIBS   ON)
		endif (${GEN_BOOST_STATIC_LIB})
		
		set(GEN_BOOST_MT_LIB TRUE CACHE BOOL "You have generated static boost lib")
		if (${GEN_BOOST_MT_LIB})
			set(Boost_USE_MULTITHREADED   ON)
		endif (${GEN_BOOST_MT_LIB})
		
		find_package(Boost REQUIRED COMPONENTS ${ARGN})
	endif (${UNIX})
	
	set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} PARENT_SCOPE)
	set(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIRS} PARENT_SCOPE)
	set(Boost_LIBRARIES ${Boost_LIBRARIES} PARENT_SCOPE)
	set(Boost_FOUND ${Boost_FOUND} PARENT_SCOPE)
endfunction(find_boost_lib)

function(find_sdl2_lib)
	set(SDL2_DIR "${PROJECT_SOURCE_DIR}" CACHE PATH "The root directory of SDL 2")
	set(SDL2_BINARY_DIR "${SDL2_DIR}/bin" CACHE PATH "The directory where binaries have being generated")
	find_library(SDL2_LIBRARIES_RELEASE , "SDL2" HINTS "${SDL2_BINARY_DIR}/Release/" PATHS "${SDL2_BINARY_DIR}/Release/")
	find_library(SDL2_LIBRARIES_DEBUG , "SDL2" HINTS "${SDL2_BINARY_DIR}/Debug/" PATHS "${SDL2_BINARY_DIR}/Debug/")
	find_library(SDL2MAIN_LIBRARIES_RELEASE , "SDL2main" HINTS "${SDL2_BINARY_DIR}/Release/" PATHS "${SDL2_BINARY_DIR}/Release/")
	find_library(SDL2MAIN_LIBRARIES_DEBUG , "SDL2main" HINTS "${SDL2_BINARY_DIR}/Debug/" PATHS "${SDL2_BINARY_DIR}/Debug/")
	
	if (NOT(SDL2_LIBRARIES_DEBUG STREQUAL "SDL2_LIBRARIES_DEBUG-NOTFOUND")
	AND NOT(SDL2_LIBRARIES_RELEASE STREQUAL "SDL2_LIBRARIES_RELEASE-NOTFOUND"))
		set(SDL_LIBRARY optimized ${SDL2_LIBRARIES_RELEASE} optimized ${SDL2MAIN_LIBRARIES_RELEASE} debug ${SDL2_LIBRARIES_DEBUG} debug ${SDL2MAIN_LIBRARIES_DEBUG} PARENT_SCOPE)
		set(SDL_FOUND true PARENT_SCOPE)
		set(SDL_INCLUDE_DIR "${SDL2_DIR}/include" PARENT_SCOPE)
		set(SDL_VERSION_STRING "2.0" PARENT_SCOPE)
	else (NOT(SDL2_LIBRARIES_DEBUG STREQUAL "SDL2_LIBRARIES_DEBUG-NOTFOUND")
	AND NOT(SDL2_LIBRARIES_RELEASE STREQUAL "SDL2_LIBRARIES_RELEASE-NOTFOUND"))
		set (SDL_FOUND false PARENT_SCOPE)
		message(WARNING "SDL 2 not found !")
		message(WARNING "SDL2_DIR (${SDL2_DIR})")
		message(WARNING "SDL2_BINARY_DIR (${SDL2_BINARY_DIR})")
	endif(NOT(SDL2_LIBRARIES_DEBUG STREQUAL "SDL2_LIBRARIES_DEBUG-NOTFOUND")
	AND NOT(SDL2_LIBRARIES_RELEASE STREQUAL "SDL2_LIBRARIES_RELEASE-NOTFOUND"))
endfunction(find_sdl2_lib)

function(assert_exists PATH_TO_CHECK)
	if (NOT EXISTS "${PATH_TO_CHECK}")
		message (SEND_ERROR "${PATH_TO_CHECK} not found")
	endif (NOT EXISTS "${PATH_TO_CHECK}")
endfunction(assert_exists)

function(find_jsoncpp_lib)
	set(JSONCPP_DIR "${PROJECT_SOURCE_DIR}" CACHE PATH "The root directory of Jsoncpp")
	set(JSONCPP_BINARY_DIR "${JSONCPP_DIR}/bin" CACHE PATH "The directory where binaries have being generated")
	find_library(JSONCPP_LIBRARIES_RELEASE , "JSONCPP" HINTS "${JSONCPP_BINARY_DIR}/src/lib_json/Release/" PATHS "${JSONCPP_BINARY_DIR}/src/lib_json/Release/")
	find_library(JSONCPP_LIBRARIES_DEBUG , "JSONCPP" HINTS "${JSONCPP_BINARY_DIR}/src/lib_json/Debug/" PATHS "${JSONCPP_BINARY_DIR}/src/lib_json/Debug/")
	
	if (NOT(JSONCPP_LIBRARIES_DEBUG STREQUAL "JSONCPP_LIBRARIES_DEBUG-NOTFOUND")
	AND NOT(JSONCPP_LIBRARIES_RELEASE STREQUAL "JSONCPP_LIBRARIES_RELEASE-NOTFOUND"))
		set(JSONCPP_LIBRARY optimized ${JSONCPP_LIBRARIES_RELEASE} debug ${JSONCPP_LIBRARIES_DEBUG} PARENT_SCOPE)
		set(JSONCPP_FOUND true PARENT_SCOPE)
		set(JSONCPP_INCLUDE_DIR "${JSONCPP_DIR}/include" PARENT_SCOPE)
		set(SDL_VERSION_STRING "2.0" PARENT_SCOPE)
	else (NOT(JSONCPP_LIBRARIES_DEBUG STREQUAL "JSONCPP_LIBRARIES_DEBUG-NOTFOUND")
	AND NOT(JSONCPP_LIBRARIES_RELEASE STREQUAL "JSONCPP_LIBRARIES_RELEASE-NOTFOUND"))
		set (JSONCPP_LIBRARY false PARENT_SCOPE)
		message(WARNING "JSONCPP not found !")
		message(WARNING "JSONCPP_DIR (${JSONCPP_DIR})")
		message(WARNING "JSONCPP_LIBRARY (${JSONCPP_BINARY_DIR})")
	endif(NOT(JSONCPP_LIBRARIES_DEBUG STREQUAL "JSONCPP_LIBRARIES_DEBUG-NOTFOUND")
	AND NOT(JSONCPP_LIBRARIES_RELEASE STREQUAL "JSONCPP_LIBRARIES_RELEASE-NOTFOUND"))
endfunction(find_jsoncpp_lib)