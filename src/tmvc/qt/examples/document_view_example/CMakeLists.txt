

add_executable(document_view_example
               application.cpp
               application.hpp
               main.cpp
               main_window.cpp
               main_window.hpp)

target_link_libraries(document_view_example PRIVATE tmvc-qt)
