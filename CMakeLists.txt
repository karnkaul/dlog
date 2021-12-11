cmake_minimum_required(VERSION 3.14)
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

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:DLOG_DEBUG>)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} SYSTEM
  INTERFACE
    include
  PUBLIC
    ext/fmt/include
)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
target_sources(${PROJECT_NAME} PRIVATE
  include/dumb_log/channel_flags.hpp
  include/dumb_log/dumb_log.hpp
  include/dumb_log/file_logger.hpp
  include/dumb_log/level.hpp
  include/dumb_log/pipe.hpp
)
add_subdirectory(src)
get_target_property(sources ${PROJECT_NAME} SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(dlog PRIVATE /MP)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(dlog PRIVATE -Wextra -Wall -Werror=return-type $<$<NOT:$<CONFIG:Debug>>:-Werror>)
endif()
