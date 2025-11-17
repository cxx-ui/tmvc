

add_executable(rich_text_view_example
               application.hpp
               main.cpp
               main_window.cpp
               main_window.hpp)

target_link_libraries(rich_text_view_example tmvc-qt)
