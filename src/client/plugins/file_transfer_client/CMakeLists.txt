project(plugin_file_transfer_client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

set(PLUGIN_SRCS
        ${FILE_TRANSFER_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/file_transfer_plugin.cpp
)

qt_add_resources(RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/resources.qrc)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS} ${RESOURCES})
target_compile_definitions(${PROJECT_NAME} PRIVATE TC_FILE_TRANS_DLL_EXPORTS)
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_message tc_client_plugin)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg tc_message tc_common_new tc_qt_widget tc_sdk Dwmapi.lib)