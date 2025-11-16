add_subdirectory(interface)
add_subdirectory(opensource)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SKIN_LOADER_SRCS
    skin_loader.cpp
)

add_library(skin_loader ${SKIN_LOADER_SRCS} ${RESOURCES})
target_link_libraries(skin_loader PRIVATE Qt6::Widgets Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
    tc_message tc_common_new tc_qt_widget skin_interface
)