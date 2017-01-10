cmake_minimum_required (VERSION 3.7)

#input: a list of the boost library needed, can be empty
#output: Boost_LIBRARY_DIRS to be used with link_directories
#output: Boost_INCLUDE_DIRS to be used with include_directories
#output: Boost_LIBRARIES to be used with target_link_libraries
function(find_boost_lib)
	set (BOOST_FOUND 0)
	
	if (${UNIX} OR ${APPLE})
		find_package(Boost REQUIRED COMPONENTS ${ARGN})
	else (${UNIX} OR ${APPLE})
		set(BOOST_ROOT ${PROJECT_SOURCE_DIR} CACHE STRING "The root directory of boost, leave it as is if you initialized subrepositories")
		set (BOOST_VERSION "1_63" CACHE STRING "The version of boost compiled")
		if (EXISTS "${BOOST_ROOT}")
			set (BOOST_FOUND 1)
		else (EXISTS "${BOOST_ROOT}")
			message (SEND_ERROR "${BOOST_ROOT} not found")
		endif (EXISTS "${BOOST_ROOT}")
		
		foreach (LIB ${ARGN})
			find_library(Boost_LIBRARIES_RELEASE_${LIB} "libboost_${LIB}-vc-mt-${BOOST_VERSION}" HINTS "${BOOST_ROOT}/stage/lib" PATHS "${BOOST_ROOT}/stage/lib")
			find_library(Boost_LIBRARIES_DEBUG_${LIB} "libboost_${LIB}-vc-mt-gd-${BOOST_VERSION}" HINTS "${BOOST_ROOT}/stage/lib" PATHS "${BOOST_ROOT}/stage/lib")
			
			if ((${Boost_LIBRARIES_RELEASE_${LIB}} STREQUAL "Boost_LIBRARIES_RELEASE_${LIB}-NOTFOUND") OR (${Boost_LIBRARIES_DEBUG_${LIB}} STREQUAL "Boost_LIBRARIES_DEBUG_${LIB}-NOTFOUND"))
				set (BOOST_FOUND 0)
				message (SEND_ERROR "lib ${LIB} not found")
			else ((${Boost_LIBRARIES_RELEASE_${LIB}} STREQUAL "Boost_LIBRARIES_RELEASE_${LIB}-NOTFOUND") OR (${Boost_LIBRARIES_DEBUG_${LIB}} STREQUAL "Boost_LIBRARIES_DEBUG_${LIB}-NOTFOUND"))
				message ("${Boost_LIBRARIES_RELEASE_${LIB}} found")
				message ("${Boost_LIBRARIES_DEBUG_${LIB}} found")
				list (APPEND Boost_LIBRARIES_RELEASE optimized)
				list (APPEND Boost_LIBRARIES_RELEASE "${Boost_LIBRARIES_RELEASE_${LIB}}")
				list (APPEND Boost_LIBRARIES_DEBUG debug)
				list (APPEND Boost_LIBRARIES_DEBUG "${Boost_LIBRARIES_DEBUG_${LIB}}")
			endif ((${Boost_LIBRARIES_RELEASE_${LIB}} STREQUAL "Boost_LIBRARIES_RELEASE_${LIB}-NOTFOUND") OR (${Boost_LIBRARIES_DEBUG_${LIB}} STREQUAL "Boost_LIBRARIES_DEBUG_${LIB}-NOTFOUND"))		
		endforeach (LIB)		
		
		set (Boost_LIBRARY_DIRS "${BOOST_ROOT}/stage/lib" PARENT_SCOPE)
		set (Boost_LIBRARIES ${Boost_LIBRARIES_DEBUG} ${Boost_LIBRARIES_RELEASE} PARENT_SCOPE)
		set (Boost_INCLUDE_DIRS "${BOOST_ROOT}" PARENT_SCOPE)
	endif (${UNIX} OR ${APPLE})
endfunction(find_boost_lib)

	set(SDL_LIBRARY optimized ${SDL2_LIBRARIES_RELEASE} debug ${SDL2_LIBRARIES_DEBUG} PARENT_SCOPE)
function(assert_exists PATH_TO_CHECK)
	if (NOT EXISTS "${PATH_TO_CHECK}")
		message (SEND_ERROR "${PATH_TO_CHECK} not found")
	endif (NOT EXISTS "${PATH_TO_CHECK}")
endfunction(assert_exists)