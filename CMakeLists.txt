# MyProject
cmake_minimum_required(VERSION 3.15)
project(Threadpool VERSION 0.1.0 LANGUAGES C CXX)

cmake_policy(SET CMP0074 NEW)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH} )

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set (CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/deploy" CACHE PATH "default install path" FORCE )
endif()

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(test "Build all tests." OFF) # Makes boolean 'test' available.

SET(MY_PROJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# force static runtime libraries for msvc builds
if(MSVC)
  set(_flag_variables
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS_MINSIZEREL
  )
  foreach(_flag_variable ${_flag_variables})
    string(REGEX REPLACE "/MT" "/MD" ${_flag_variable} "${${_flag_variable}}")
  endforeach()
endif()

# ---
# This subfolder needs to run in compile time
add_subdirectory(thirdparty)
add_subdirectory(src)

# ---
add_subdirectory(tests)

