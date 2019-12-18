Project(MyLibrary CXX)

include(assign_source_group)

set(LIBRARY TaskScheduler)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.c *.inc)
file(GLOB_RECURSE HEADER_FILES *.h *.hpp *.inl)

add_library(${LIBRARY} STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(${LIBRARY}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${SPDLOG_INCLUDE_DIR}
  )

target_link_directories(${LIBRARY} PUBLIC
  ${SPDLOG_LIB_DIR}
  )

target_link_libraries(${LIBRARY}
  ${SPDLOG_LIBRARIES}
  Threads::Threads
  )

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /Ob1")
endif (MSVC)

set_target_properties(${LIBRARY} PROPERTIES DEBUG_POSTFIX "d")

assign_source_group(${SOURCE_FILES} ${HEADER_FILES})

install(TARGETS ${LIBRARY} DESTINATION lib)
install(DIRECTORY / DESTINATION include FILES_MATCHING PATTERN "*.h")
install(DIRECTORY / DESTINATION include FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY / DESTINATION include FILES_MATCHING PATTERN "*.inl")

