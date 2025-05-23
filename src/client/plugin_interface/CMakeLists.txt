project(tc_client_plugin)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Svg)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/ct_plugin_interface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ct_plugin_context.cpp
)

add_library(${PROJECT_NAME} ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network Qt6::Core Qt6::Core5Compat
        Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg tc_common_new)