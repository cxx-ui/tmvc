
cmake_minimum_required(VERSION 3.15)
project(tmvc)

option(TMVC_BUILD_DOC "Build documentation" OFF)

set(CMAKE_CXX_STANDARD 20)


# Use boost provided config files
cmake_policy(SET CMP0167 NEW)
find_package(Boost 1.70 REQUIRED COMPONENTS unit_test_framework)


add_subdirectory(src)

if("${TMVC_BUILD_DOC}")
    add_subdirectory(doc)
endif()
