project(GammaRayRender VERSION 0.1 LANGUAGES CXX)

add_subdirectory(settings)
add_subdirectory(plugins)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network)
find_package(OpenCV CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/3rd)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/3rd/asio)

set(SRCS
        app.cpp
        context.cpp
        statistics.cpp
        network/app_server.cpp
        network/ws_media_router.cpp
        network/http_handler.cpp
        network/ws_ipc_router.cpp
        network/connection.cpp
        network/message_processor.cpp
        network/ws_client.cpp
        network/net_message_maker.cpp
        network/server_cast.cpp
        ipc/host_ipc_manager.cpp
        app/encoder_thread.cpp
        plugins/plugin_manager.cpp
        plugins/plugin_event_router.cpp
        plugins/plugin_stream_event_router.cpp
        plugins/plugin_net_event_router.cpp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/webrtc)

add_library(main ${SRCS})

if(WIN32)
    target_sources(main PRIVATE
            ${WIN32_SRCS}
    )
else()
    #
endif()

target_link_libraries(main app_manager tc_message Dexode::EventBus)
target_link_libraries(main Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network)

add_executable(${PROJECT_NAME} WIN32 render_main.cpp)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
target_link_libraries(${PROJECT_NAME} settings main tc_capture_new tc_common_new tc_message tc_encoder_new tc_steam_manager_new tc_vigem legacy_encoder tc_plugin) # tc_rtc
target_link_libraries(${PROJECT_NAME} sqlite_orm EasyHook64 gflags::gflags leveldb)
target_link_libraries(${PROJECT_NAME} opencv_ml opencv_dnn opencv_core opencv_flann)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/settings.toml"  "${CMAKE_BINARY_DIR}"
        COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avcodec-59.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avdevice-59.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avfilter-8.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avutil-57.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avformat-59.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/postproc-56.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/swresample-4.dll"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/swscale-6.dll"  "${CMAKE_BINARY_DIR}"
)

