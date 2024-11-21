project(plugin_nvenc_encoder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NVSDK_VERSION
        12
)

include(nvencoder/${NVSDK_VERSION}/CMakeLists.txt)

include_directories(
        nvencoder/${NVSDK_VERSION}
)

set(NVENCODER_SOURCES_LIST)
foreach(item ${NVENCODER_SOURCES})
    #message("Current item: ${item}")
    list(APPEND NVENCODER_SOURCES_LIST "${item}")
endforeach()

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/nvenc_encoder_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/nvenc_video_encoder.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS} ${NVENCODER_SOURCES_LIST})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_message
        tc_plugin sqlite_orm winmm.lib SetupAPI.lib)