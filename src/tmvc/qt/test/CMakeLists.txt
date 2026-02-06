
add_executable(tmvc-qt-test
               formatted_char_test.cpp
               test.cpp)

target_link_libraries(tmvc-qt-test PRIVATE tmvc-qt Boost::unit_test_framework)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(tmvc-qt-test PRIVATE rt)
endif()

add_test(NAME tmvc-qt-test COMMAND tmvc-qt-test)
