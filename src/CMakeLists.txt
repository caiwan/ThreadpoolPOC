include(project_warnings)

include(AssignSourceGroup)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCE_FILES
    ThreadPool/*.cpp
    ThreadPool/*.c
    ThreadPool/*.inc
    )


file(GLOB_RECURSE HEADER_FILES
    ThreadPool/*.h
    ThreadPool/*.hpp
    ThreadPool/*.inl
    )

add_library(ThreadPool STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(ThreadPool PRIVATE
    spdlog
    Threads::Threads
    project_warnings
    )

target_include_directories(
    ThreadPool
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(ThreadPool PROPERTIES DEBUG_POSTFIX "d")

target_include_directories(ThreadPool
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    )


assign_source_group(${SOURCE_FILES} ${HEADER_FILES})

install(TARGETS ThreadPool DESTINATION lib)
install(DIRECTORY / DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY / DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY / DESTINATION include FILES_MATCHING PATTERN "*.inl")
