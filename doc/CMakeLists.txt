
find_package(Doxygen REQUIRED)

set(DOXYGEN_RECURSIVE TRUE)
set(DOXYGEN_PROJECT_NAME "TMVC")
set(DOXYGEN_PROJECT_BRIEF "Generic C++ Text Model-View-Controller library")
set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/index.md")
doxygen_add_docs(tmvc-docs ALL
                 "${CMAKE_CURRENT_SOURCE_DIR}/../src"
                 "${CMAKE_CURRENT_SOURCE_DIR}")
