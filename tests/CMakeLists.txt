enable_testing()

include(GoogleTest)
include("deploy_tests.cmake")

file(GLOB_RECURSE SOURCE_FILES *.cpp *.c)
file(GLOB_RECURSE HEADER_FILES *.hpp *.h *.hh)

add_executable(UnitTests EXCLUDE_FROM_ALL ${SOURCE_FILES} ${HEADER_FILES})

message("target_include_directories(UnitTests PUIBLIC ${GTEST_INCLUDE_DIR})")

target_link_libraries(UnitTests PUBLIC
    ThreadPool
  gtest gtest_main gmock
  spdlog
  )

set_target_properties(UnitTests PROPERTIES FOLDER tests)

add_test(NAME Tests COMMAND UnitTests WORKING_DIRECTORY ${TEST_DEPLOY_DIR})

add_dependencies(UnitTests DeployTests)
