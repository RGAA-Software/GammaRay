project(plugin_frame_resizer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/frame_resizer_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/frame_render.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_message tc_plugin sqlite_orm winmm.lib SetupAPI.lib)