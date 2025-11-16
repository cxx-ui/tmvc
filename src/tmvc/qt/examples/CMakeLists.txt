
option(TMVC_QT_ENABLE_EXAMPLES "Enable TMVC library Qt examples" ON)

if(NOT "${TMVC_QT_ENABLE_EXAMPLES}")
    return()
endif()

add_subdirectory(text_view_example)
add_subdirectory(document_view_example)
