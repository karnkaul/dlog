cmake_minimum_required(VERSION 3.14)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "")

set(project_name dlog)
set(${project_name}_version 1.0.1)
set(cxx_standard cxx_std_17)

project(${project_name} VERSION ${${project_name}_version})
set(is_root_project OFF) # indicate if this is the top-level project

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(is_root_project ON)
  message(STATUS "${PROJECT_NAME} is top level")
endif()

# options
option(DLOG_OUTPUT_DEBUG_STRING "Convert to wide-char and call OutputDebugString if debugger present (MSVC only)" ON)
option(DLOG_INSTALL "Install dlog" ${is_root_project})

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

set(FMT_INSTALL ${DLOG_INSTALL})
add_subdirectory(ext/fmt)

add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:DLOG_DEBUG> $<$<BOOL:${DLOG_OUTPUT_DEBUG_STRING}>:DLOG_OUTPUT_DEBUG_STRING>)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} SYSTEM
  INTERFACE
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ext/fmt/include>"
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

if(DLOG_INSTALL)
  # cmake-utils
  include(FetchContent)
  set(FETCHCONTENT_QUIET OFF)
  FetchContent_Declare(
    cmake-utils
    GIT_REPOSITORY https://github.com/karnkaul/cmake-utils
    GIT_TAG v1.0.1
  )
  FetchContent_MakeAvailable(cmake-utils)
  FetchContent_GetProperties(cmake-utils)

  include("${cmake-utils_SOURCE_DIR}/cmake-utils.cmake")
  install(TARGETS ${PROJECT_NAME} fmt EXPORT ${PROJECT_NAME}-targets)
  install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )
  install(DIRECTORY include/ ext/fmt/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h*"
  )
  install_package_config(PACKAGE ${PROJECT_NAME})
  install_package_version(PACKAGE ${PROJECT_NAME} VERSION ${${PROJECT_NAME}_version})
  export_package_to_build_tree(PACKAGE ${PROJECT_NAME})
endif()
