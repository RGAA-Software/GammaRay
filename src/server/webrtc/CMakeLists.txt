cmake_minimum_required(VERSION 3.26)

project(tc_rtc)

set(WEBRTC_ROOT ${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/webrtc/4844)
include_directories(${WEBRTC_ROOT}/)
include_directories(${WEBRTC_ROOT}/include)
include_directories(${WEBRTC_ROOT}/include/third_party/abseil-cpp)
include_directories(${WEBRTC_ROOT}/include/third_party/jsoncpp/source/include/)
include_directories(${WEBRTC_ROOT}/include/base)
include_directories(.)
if(MSVC)
    add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

if (WIN32)
    add_definitions(-DWEBRTC_WIN=1 -DWEBRTC_USE_H264=1)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    add_definitions(-DQT_DEPRECATED_WARNINGS -DQT_NO_KEYWORDS)
    link_directories(${WEBRTC_ROOT}/lib/win64)
elseif (APPLE)

elseif (ANDROID)
elseif (UNIX)
endif ()

set(SRCS
    rtc_server.cpp
)

add_library(${PROJECT_NAME} SHARED ${SRCS})
add_library(${PROJECT_NAME}_lib ${SRCS})

if (WIN32)
    target_link_libraries(${PROJECT_NAME} ${WEBRTC_ROOT}/lib/win64/webrtc.lib
            Winmm ws2_32 Secur32 Crypt32 Msdmo dmoguids tc_common_new
            wmcodecdspuuid Iphlpapi.lib Strmiids Quartz Ole32
    )

    target_link_libraries(${PROJECT_NAME}_lib ${WEBRTC_ROOT}/lib/win64/webrtc.lib
            Winmm ws2_32 Secur32 Crypt32 Msdmo dmoguids tc_common_new
            wmcodecdspuuid Iphlpapi.lib Strmiids Quartz Ole32
    )

elseif (APPLE)

elseif (ANDROID)
elseif (UNIX)
endif ()

add_executable(test_webrtc test/test_webrtc.cpp)
target_link_libraries(test_webrtc tc_encoder_new ${PROJECT_NAME}_lib)