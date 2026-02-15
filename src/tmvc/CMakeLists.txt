
add_library(tmvc INTERFACE)
target_include_directories(tmvc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_subdirectory(test)
add_subdirectory(qt)
