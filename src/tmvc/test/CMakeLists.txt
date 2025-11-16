

# Application test
add_executable(texttest
               editor_controller_test.cpp
               line_numbers_text_model_test.cpp
               simple_text_model_test.cpp
               single_selection_model_test.cpp
               test.cpp
               text_line_numbers_model_test.cpp
               viewer_controller_test.cpp
               viewport_model_test.cpp)

target_link_libraries(texttest PRIVATE tmvc-qt Boost::unit_test_framework)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    target_link_libraries(texttest PRIVATE rt)
endif()

add_test(NAME texttest COMMAND texttest)
