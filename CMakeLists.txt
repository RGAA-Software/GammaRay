cmake_minimum_required(VERSION 3.26)
include(env_settings.cmake)

set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

#project(GammaRay VERSION 0.1 LANGUAGES CXX)
project(GammaRay)

set(BUILD_FROM "IDE" CACHE STRING "Build from")
set(BUILD_TYPE_PATH "")

message("==> CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("==> BUILD_FROM: ${BUILD_FROM}")
message("==> TC_USE_VR_MANAGER: ${TC_USE_VR_MANAGER}")

if(${BUILD_FROM} STREQUAL "CMD")
    set(BUILD_TYPE_PATH "${CMAKE_BUILD_TYPE}/")
else ()

endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

#find_package(OpenSSL REQUIRED)
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Svg)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Charts Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Charts Svg)
find_package(SDL2 CONFIG REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)

include_directories(${VK_SDK})
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/eventbus/src)
include_directories(${CMAKE_BINARY_DIR}/deps/tc_message_new)
include_directories(${CMAKE_BINARY_DIR}/deps/tc_server_protocol)
include_directories(${CMAKE_BINARY_DIR}/deps/tc_3rdparty/qwindowkit/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/3rd)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/3rd/asio)
#include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/protobuf/src)
#include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/abseil-cpp)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/sqlite_orm/include)
#include_directories(${VCPKG_ROOT}/installed/x64-windows-static/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/webrtc/include/third_party/libyuv/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/leveldb/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/qwindowkit)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/qwindowkit/src)
include_directories(${CMAKE_SOURCE_DIR}/src/render)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_qt_widget)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_client_sdk_new)

link_directories(${VCPKG_ROOT}/installed/x64-windows/lib)
link_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64)
link_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/openssl/x64/lib/VC/static)

if(MSVC)
    add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
    add_compile_options("/bigobj")
    include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/include)
    link_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64)

    include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/openssl/include)
    link_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/openssl/x64/lib/VC)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:83886080")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,83886080")
endif()

add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
#add_compile_definitions(ASIO2_ENABLE_SSL ASIO2_ENABLE_LOG)
add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DENABLE_SSL=0 -D_AMD64_)#-DCMAKE_GENERATOR_PLATFORM=x64
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

find_package(gflags CONFIG REQUIRED)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")

### CPY
file(COPY ${CMAKE_SOURCE_DIR}/src/render/settings.toml DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH})
file(COPY ${CMAKE_SOURCE_DIR}/src/render/private.key DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH})
file(COPY ${CMAKE_SOURCE_DIR}/src/render/certificate.pem DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH})
### CPY

set(ENABLE_HOOK_CAPTURE ON)

set(Protobuf_SRC_ROOT_FOLDER ${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/protobuf)
if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(Protobuf_LIBRARIES ${CMAKE_BINARY_DIR}/deps/tc_3rdparty/protobuf/cmake/libprotobuf.lib)
endif()
find_package(Protobuf REQUIRED)

set(OpenCV_ROOT ${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/opencv_410)
set(OpenCV_Libs ${OpenCV_ROOT}/x64/vc16/lib/opencv_world4100.lib)
include_directories(${OpenCV_ROOT}/include)

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(src/tests)
add_subdirectory(src/service)
add_subdirectory(src/uninstall)
add_subdirectory(src/client)

message("--==> Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message("--==> Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")

### Translation
set(TRANSLATION ON)
if(TRANSLATION)
    set(TS_DIR ${CMAKE_SOURCE_DIR}/translation)
    set(LUPDATE_EXE_PATH ${QT_ROOT}/bin/lupdate.exe)
    set(LRELEASE_EXE_PATH ${QT_ROOT}/bin/lrelease.exe)

    set(TS_FILES
        ${TS_DIR}/${PROJECT_NAME}_zh_CN.ts
        ${TS_DIR}/${PROJECT_NAME}_en_US.ts
    )
    foreach(_ts_file ${TS_FILES})
        execute_process(
                COMMAND ${LUPDATE_EXE_PATH} -recursive ${CMAKE_SOURCE_DIR}/src/ui -ts ${_ts_file})
        execute_process(
                COMMAND ${LUPDATE_EXE_PATH} ${_ts_file})
    endforeach()
endif ()
### Translation

set(RES resources/font.qrc resources/main.qrc resources/qss.qrc res.qrc)
add_executable(${PROJECT_NAME} WIN32
        main.cpp
        ${RES}
        ${MAIN_SRCS}
        ${NETWORK_SRCS}
        icon.rc
        ${TS_FILES}
        ${SERVICE_MGR_SRCS}
        ${DEVICE_LIST_SRCS}
)

#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/certificate.pem" DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/private.key" DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_client_web/" DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}www)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_message_new/" DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}www)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network Qt6::Core Qt6::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg
        tc_common_new tc_steam_manager_new tc_vigem tc_message tc_plugin tc_qt_widget sqlite_orm winmm.lib SetupAPI.lib tc_signaling_sdk #lib_GammaRayClient
        tc_relay_client tc_manager_client QWindowKit::Widgets TC_WidgetFrame tc_translator tc_spv_client)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins
)

### Service Beg
### Service
add_executable(GammaRayService ${SERVICE_SRCS})
SET_TARGET_PROPERTIES(GammaRayService PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
target_link_libraries(GammaRayService PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(GammaRayService PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message tc_plugin tc_qt_widget sqlite_orm winmm.lib SetupAPI.lib)

### Service Manager
add_executable(GammaRayServiceManager ${SERVICE_MGR_MAIN_SRCS} ${SERVICE_MGR_SRCS})
SET_TARGET_PROPERTIES(GammaRayServiceManager PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")

target_link_libraries(GammaRayServiceManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(GammaRayServiceManager PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message tc_plugin tc_qt_widget sqlite_orm winmm.lib SetupAPI.lib)
### Service End

### Uninstall
add_executable(uninstall WIN32
        icon.rc
        ${UNINSTALL_SRCS}
        ${SERVICE_MGR_SRCS})
SET_TARGET_PROPERTIES(uninstall PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
add_executable(shadow_deleter ${SHADOW_DELETER_SRCS})
SET_TARGET_PROPERTIES(shadow_deleter PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")

target_link_libraries(uninstall PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network tc_qt_widget
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(uninstall PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message tc_plugin sqlite_orm winmm.lib SetupAPI.lib)
### Uninstall End

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_target(tc_build_all ALL COMMENT "Build all targets")
add_dependencies(tc_build_all ${PROJECT_NAME} GammaRay GammaRayRender GammaRayService GammaRayServiceManager tc_message tc_vigem tc_plugin
        uninstall shadow_deleter GammaRayClientInner tc_manager_client tc_relay_client tc_rtc_client) #GammaRayClient
if(ENABLE_HOOK_CAPTURE)
    add_dependencies(tc_build_all tc_graphics tc_graphics_helper tc_graphics_util) #tc_capture_library
endif ()
#
add_dependencies(tc_build_all plugin_net_udp plugin_net_rtc plugin_net_ws plugin_media_recorder  plugin_ffmpeg_encoder
        plugin_amf_encoder plugin_frame_debugger plugin_nvenc_encoder plugin_dda_capture plugin_frame_resizer
        plugin_was_audio_capture plugin_opus_encoder tc_sdk tc_qt_widget plugin_file_transfer tc_signaling_sdk #lib_GammaRayClient
        plugin_mock_video_stream plugin_obj_detector plugin_net_relay QWindowKit::Widgets TC_WidgetFrame tc_translator tc_spv_client
        plugin_clipboard
)

add_custom_target(tc_build_client ALL COMMENT "Build client")
add_dependencies(tc_build_client GammaRayClientInner)#GammaRayClient

if(TC_USE_VR_MANAGER)
    message("===> tc_vr_plugin_builder")
    add_custom_target(tc_vr_plugin_builder)
    add_dependencies(tc_vr_plugin_builder plugin_vr_manager)
    add_custom_command(TARGET tc_vr_plugin_builder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_vr_manager.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_vr_manager.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/tc_vr_plugin/plugin_vr_manager.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_vr_manager.dll.toml"
    )
    add_dependencies(tc_build_all tc_vr_plugin_builder)
endif ()

add_custom_command(TARGET tc_build_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_controller/vigem/driver/ViGEmBus_1.22.0_x64_x86_arm64.exe"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/icon.ico"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_icon.ico"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/client/icon.ico"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_client_icon.ico"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/install.py"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}package/install.py"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/package"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}package"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/qt_material"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}resources/qt_material"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources/language"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}resources/language"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/settings.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}"
        COMMAND ${CMAKE_COMMAND} -E copy "${OpenCV_ROOT}/x64/vc16/bin/opencv_world4100.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_net_udp.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_udp.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/net_udp/plugin_net_udp.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_udp.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_net_rtc.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_rtc.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/net_rtc/plugin_net_rtc.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_rtc.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_net_ws.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_ws.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/net_ws/plugin_net_ws.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_ws.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_media_recorder.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_media_recorder.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/media_recorder/plugin_media_recorder.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_media_recorder.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_obj_detector.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_obj_detector.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/obj_detector/plugin_obj_detector.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_obj_detector.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_ffmpeg_encoder.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_ffmpeg_encoder.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/ffmpeg_encoder/plugin_ffmpeg_encoder.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_ffmpeg_encoder.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_amf_encoder.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_amf_encoder.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/amf_encoder/plugin_amf_encoder.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_amf_encoder.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_frame_debugger.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_frame_debugger.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/frame_debugger/plugin_frame_debugger.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_frame_debugger.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_nvenc_encoder.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_nvenc_encoder.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/nvenc_encoder/plugin_nvenc_encoder.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_nvenc_encoder.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_dda_capture.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_dda_capture.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/dda_capture/plugin_dda_capture.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_dda_capture.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_frame_resizer.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_frame_resizer.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/frame_resizer/plugin_frame_resizer.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_frame_resizer.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_mock_video_stream.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_mock_video_stream.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/mock_video_stream/plugin_mock_video_stream.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_mock_video_stream.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_was_audio_capture.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_was_audio_capture.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/was_audio_capture/plugin_was_audio_capture.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_was_audio_capture.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_opus_encoder.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_opus_encoder.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/opus_encoder/plugin_opus_encoder.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_opus_encoder.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_file_transfer.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_file_transfer.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/file_transfer/plugin_file_transfer.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_file_transfer.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_net_relay.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_relay.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/net_relay/plugin_net_relay.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_net_relay.dll.toml"

        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}plugin_clipboard.dll"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_clipboard.dll"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/render/plugins/clipboard/plugin_clipboard.dll.toml"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}gr_plugins/plugin_clipboard.dll.toml"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_controller/vigem/driver/ViGEmBus_1.22.0_x64_x86_arm64.exe"  "${CMAKE_BINARY_DIR}/${BUILD_TYPE_PATH}"
)