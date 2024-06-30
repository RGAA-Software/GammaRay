project(GammaRayServer VERSION 0.1 LANGUAGES CXX)

add_subdirectory(settings)

find_package(ZLIB REQUIRED)
find_package(OpenCV REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Multimedia)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# C:\source\vcpkg\installed\x64-windows-static\lib
#link_directories(${VCPKG_ROOT}/installed/x64-windows-static/lib)
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

target_link_libraries(main app_manager tc_message)
target_link_libraries(main Dexode::EventBus ZLIB::ZLIB ${OpenCV_LIBS})
target_link_libraries(main Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Multimedia)

add_executable(${PROJECT_NAME} srv_main.cpp)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
target_link_libraries(${PROJECT_NAME} settings main tc_capture_new tc_common_new tc_message tc_encoder_new tc_steam_manager_new tc_vigem) #legacy_encoder
target_link_libraries(${PROJECT_NAME} sqlite_orm EasyHook64 gflags::gflags)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/server/settings.toml"  "${CMAKE_BINARY_DIR}"
)

