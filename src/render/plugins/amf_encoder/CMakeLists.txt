project(plugin_amf_encoder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(shared)

set(EXT_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/Windows/ThreadWindows.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/AMFFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/AMFSTL.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/Thread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/TraceAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/CurrentTimeImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/DataStreamFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/DataStreamFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/DataStreamMemory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/IOCapsImpl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/amf/common/PropertyStorageExImpl.cpp
)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/amf_encoder_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/d3d_texture_debug.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/encode_debug_callback.h
        ${CMAKE_CURRENT_SOURCE_DIR}/video_encoder_vce.cpp
)

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS} ${EXT_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_plugin winmm.lib amf_encoder_shared SetupAPI.lib)