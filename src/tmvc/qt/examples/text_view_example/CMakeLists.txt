

add_executable(text_view_example
               application.hpp
               main.cpp
               main_window.cpp
               main_window.hpp)

target_link_libraries(text_view_example tmvc-qt)
