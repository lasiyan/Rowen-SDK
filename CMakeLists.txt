# -- Default setting -------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(RS_PROJECT VERSION 2.0)

include(cmake/compiler.cmake)
include(cmake/define.cmake)
include(cmake/macro.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
# --------------------------------------------------------------------------------

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(_DEBUG)
  #set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fsanitize=address)
endif()


# -- modules ---------------------------------------------------------------------
add_subdirectory(modules/core)
add_subdirectory(modules/utils)

# -- samples ---------------------------------------------------------------------
add_subdirectory(sample/example-time)
add_subdirectory(sample/example-logger)
add_subdirectory(sample/example-threadpool)
add_subdirectory(sample/example-resource)

# -- post install event ----------------------------------------------------------
# include(cmake/post-install.cmake)
