# Third party packages

# https://github.com/TheLartians/CPM.cmake
include(CPM)

CPMAddPackage(
  NAME googletest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.8.1
  VERSION 1.8.1
  OPTIONS
  "INSTALL_GTEST OFF"
  "gtest_force_shared_crt ON"
)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  GIT_TAG v1.4.2
  VERSION 1.4.2
)
