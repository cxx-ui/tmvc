
add_library(tmvc INTERFACE)
target_include_directories(tmvc INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_link_libraries(tmvc INTERFACE Boost::boost)

add_subdirectory(test)
add_subdirectory(qt)
