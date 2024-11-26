project(plugin_mock_video_stream VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV CONFIG REQUIRED)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/mock_video_stream_plugin.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_message tc_plugin sqlite_orm winmm.lib SetupAPI.lib)
# note: 10 additional targets are not displayed.
message("xxx opencv libs: ${OpenCV_LIBS}")
target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})