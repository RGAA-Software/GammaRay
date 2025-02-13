project(plugin_dda_capture)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/dda_capture_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cursor_capture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/dda_capture.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/desktop_capture.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_message tc_plugin sqlite_orm winmm.lib SetupAPI.lib)