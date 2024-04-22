project(GammaRay VERSION 0.1 LANGUAGES CXX)

add_subdirectory(settings)

find_package(ZLIB REQUIRED)
find_package(OpenCV REQUIRED)
find_package(websocketpp CONFIG REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# C:\source\vcpkg\installed\x64-windows-static\lib
link_directories(${VCPKG_ROOT}/installed/x64-windows-static/lib)

set(SRCS
    app.cpp
    context.cpp
    statistics.cpp
    server_monitor.cpp
    network/app_server.cpp
    network/ws_media_router.cpp
    network/http_handler.cpp
    network/session.cpp
    network/ws_server.cpp
    network/ws_ipc_router.cpp
    network/connection.cpp
    network/message_processor.cpp
    network/ws_client.cpp
    ipc/host_ipc_manager.cpp
    app/encoder_thread.cpp
    network/message_maker.cpp
    app/command_manager.cpp
    network/server_cast.cpp
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/app)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/webrtc)

add_library(main ${SRCS})

if(WIN32)
    target_sources(main PRIVATE
            ${WIN32_SRCS}
    )
else()
    #
endif()

target_link_libraries(main app_manager tc_message tc_dyn_rtc)
target_link_libraries(main Dexode::EventBus ZLIB::ZLIB ${OpenCV_LIBS} PocoFoundationmt.lib)
target_link_libraries(main websocketpp::websocketpp)

add_executable(GammaRayServer srv_main.cpp)
SET_TARGET_PROPERTIES(GammaRayServer PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
target_link_libraries(GammaRayServer settings main tc_capture_new tc_common_new tc_message tc_encoder_new tc_steam_manager_new tc_vigem) #legacy_encoder
target_link_libraries(GammaRayServer sqlite_orm::sqlite_orm EasyHook64 gflags::gflags)
