include(ExternalProject)

SET(PREFIX spdlog)
SET(GIT_URL https://github.com/gabime/spdlog.git)
SET(GIT_TAG "79259fdb3f57283f65185a8e41840968dae4043c")

SET(SPDLOG_INSTALL_DIR ${CMAKE_BINARY_DIR}/externals/)

set(SPDLOG_BUILD_SHARED_LIBS OFF)

ExternalProject_Add(${PREFIX}
  GIT_REPOSITORY "${GIT_URL}"
  GIT_TAG "${GIT_TAG}"
  PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${GTEST_INSTALL_DIR} -DBUILD_SHARED_LIBS=${GTEST_BUILD_SHARED_LIBS}
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  )

SET(SPDLOG_INCLUDE_DIR "${SPDLOG_INSTALL_DIR}/include" CACHE PATH "")

if (WIN32)
  # NOTE: MSVC only
  SET(_lib_dir "lib")
  SET(_lib_prefix "")
  SET(_lib_suffix ".lib")
  SET(_bin_dir "bin")
  SET(_bin_prefix "")
  SET(_bin_suffix ".dll")
elseif (UNIX AND NOT APPLE)
  SET(_lib_dir "lib")
  SET(_lib_prefix "lib")
  SET(_lib_suffix ".a")
  SET(_bin_prefix "lib")
  SET(_bin_suffix ".so")
elseif (APPLE)
  SET(_lib_dir "lib")
  SET(_lib_prefix "")
  SET(_lib_suffix "")
  SET(_bin_dir "lib")
  SET(_bin_prefix "")
  SET(_bin_suffix ".dylib")
endif ()

# Glue all together

if (GENERATOR_IS_MULTI_CONFIG)
  set(SPDLOG_LIBRARIES
    debug
    "${_lib_prefix}spdlog${CMAKE_DEBUG_POSTFIX}${_lib_suffix}"
    optimized
    "${_lib_prefix}spdlog${CMAKE_RELEASE_POSTFIX}${_lib_suffix}"
    CACHE INTERNAL "" FORCE)

else (GENERATOR_IS_MULTI_CONFIG)
  set(SPDLOG_LIBRARIES
    "${_lib_prefix}spdlog${_lib_suffix}"
    CACHE INTERNAL "" FORCE)

endif (GENERATOR_IS_MULTI_CONFIG)

SET(SPDLOG_LIB_DIR "${SPDLOG_INSTALL_DIR}/${_lib_dir}" CACHE PATH "" FORCE)
SET(SPDLOG_BIN_DIR "${SPDLOG_INSTALL_DIR}/${_bin_dir}" CACHE PATH "" FORCE)

