
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

option(TMVC_BUILD_EXAMPLES "Build tmvc Qt examples" OFF)

if("${TMVC_USE_QT5}")
    set(qt_name "Qt5")
else()
    set(qt_name "Qt6")
endif()

find_package(${qt_name} COMPONENTS "Widgets")

add_library(tmvc-qt INTERFACE)

target_link_libraries(tmvc-qt INTERFACE tmvc)
target_link_libraries(tmvc-qt INTERFACE ${qt_name}::Widgets)

if(TMVC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
add_subdirectory(test)
