project(plugin_ffmpeg_encoder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${FFMPEG_ROOT}include)
link_directories(${FFMPEG_ROOT}/lib)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_encoder_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg_encoder.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_message tc_plugin)
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_ROOT}/lib/avcodec.lib ${FFMPEG_ROOT}/lib/avdevice.lib ${FFMPEG_ROOT}/lib/avfilter.lib
        ${FFMPEG_ROOT}/lib/avformat.lib ${FFMPEG_ROOT}/lib/avutil.lib ${FFMPEG_ROOT}/lib/swresample.lib ${FFMPEG_ROOT}/lib/swscale.lib
        ${FFMPEG_ROOT}/lib/postproc.lib tc_opus_codec_new yuv)#${FFMPEG_LIBRARIES} frame_render