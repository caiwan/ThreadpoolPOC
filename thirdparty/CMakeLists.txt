include(${CMAKE_CURRENT_SOURCE_DIR}/gtest.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/spdlog.cmake)

add_custom_target(ExternalDependencies)
add_dependencies(ExternalDependencies
    googletest
    spdlog
)

set_target_properties(ExternalDependencies PROPERTIES FOLDER ExternalProjectTargets)
