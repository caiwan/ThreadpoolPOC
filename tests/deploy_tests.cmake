set(TEST_DEPLOY_TARGET DeployTests)
set(TEST_DEPLOY_DIR ${CMAKE_BINARY_DIR}/tests/environment)

add_custom_target(${TEST_DEPLOY_TARGET})

set_target_properties(DeployTests PROPERTIES FOLDER tests)

add_dependencies(DeployTests ExternalDependencies)
