project(plugin_net_ws)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/ws_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ws_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ws_stream_router.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ws_filetransfer_router.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/http_handler.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_net_plugin libprotobuf tc_message)