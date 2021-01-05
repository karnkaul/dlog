cmake_minimum_required(VERSION 3.3)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")

project(dlog)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
	find_package(Git QUIET)
		if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
		message(STATUS "Updating git submodules...")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			RESULT_VARIABLE UPDATE_SUBMODULES_RESULT
		)
		if(NOT UPDATE_SUBMODULES_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update failed!")
		endif()
	endif()
endif()

add_subdirectory(ext/fmt)

# -fPIC for fmt
set_property(TARGET fmt PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_features(fmt PRIVATE cxx_std_17)
if(MSVC_RUNTIME AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	target_compile_definitions(fmt PUBLIC FMT_EXCEPTIONS=0)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.?pp")
file(GLOB_RECURSE INCLUDES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.?pp")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${INCLUDES} ${SOURCES})

add_library(dlog ${INCLUDES} ${SOURCES})
add_library(dlog::dlog ALIAS dlog)
target_compile_definitions(dlog PUBLIC $<$<CONFIG:Debug>:DLOG_DEBUG>)
target_compile_features(dlog PUBLIC cxx_std_17)
target_include_directories(dlog PUBLIC include)
target_include_directories(dlog SYSTEM PUBLIC ext/fmt/include)
target_link_libraries(dlog PUBLIC fmt)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	target_compile_options(dlog PRIVATE -fPIC)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(dlog PRIVATE /MP)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(dlog PRIVATE -Wextra -Wall -Werror=return-type $<$<NOT:$<CONFIG:Debug>>:-Werror>)
endif()
