project(tc_plugin VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Svg)

set(PLUGIN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gr_plugin_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gr_stream_plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gr_encoder_plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gr_plugin_context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gr_net_plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gr_data_provider_plugin.cpp
)

add_library(${PROJECT_NAME} ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message sqlite_orm winmm.lib SetupAPI.lib)