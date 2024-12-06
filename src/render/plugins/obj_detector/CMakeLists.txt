project(plugin_obj_detector VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV CONFIG REQUIRED)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/obj_detector_plugin.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_message tc_plugin sqlite_orm winmm.lib SetupAPI.lib)
# note: 10 additional targets are not displayed.
target_link_libraries(${PROJECT_NAME} PRIVATE opencv_ml opencv_dnn opencv_core opencv_flann)