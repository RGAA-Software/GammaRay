project(plugin_ffmpeg_encoder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FFMPEG_LIB_PATH ${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/include)
link_directories(${FFMPEG_LIB_PATH})

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_encoder_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_encoder.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_steam_manager_new tc_message tc_plugin sqlite_orm winmm.lib SetupAPI.lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIB_PATH}/avcodec.lib ${FFMPEG_LIB_PATH}/avdevice.lib ${FFMPEG_LIB_PATH}/avfilter.lib
        ${FFMPEG_LIB_PATH}/avformat.lib ${FFMPEG_LIB_PATH}/avutil.lib ${FFMPEG_LIB_PATH}/swresample.lib ${FFMPEG_LIB_PATH}/swscale.lib
        ${FFMPEG_LIB_PATH}/postproc.lib tc_opus_codec_new yuv)#${FFMPEG_LIBRARIES} frame_render