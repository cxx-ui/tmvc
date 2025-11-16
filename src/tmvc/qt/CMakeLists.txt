
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if("${TMVC_USE_QT5}")
    set(qt_name "Qt5")
else()
    set(qt_name "Qt6")
endif()

find_package(${qt_name} COMPONENTS "Widgets")

add_library(tmvc-qt INTERFACE)

target_link_libraries(tmvc-qt INTERFACE tmvc)
target_link_libraries(tmvc-qt INTERFACE ${qt_name}::Widgets)

add_subdirectory(examples)
