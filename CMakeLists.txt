cmake_minimum_required(VERSION 3.26)
include(config.cmake)
include(env_settings.cmake)
include(version.cmake)

set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(GammaRay VERSION ${TC_APP_VERSION} LANGUAGES CXX)

set(BUILD_FROM "IDE" CACHE STRING "Build from")

message("==> CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("==> BUILD_FROM: ${BUILD_FROM}")
message("==> QT_ROOT:" ${QT_ROOT})
message("==> VCPKG_ROOT:" ${VCPKG_ROOT})

set(GR_PROJECT_PATH "")
set(GR_PROJECT_BINARY_PATH "")

set(NSIS_VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/package/proj_version.nsh")
file(WRITE ${NSIS_VERSION_FILE}
"!define PRODUCT_VERSION \"${TC_APP_VERSION}\"\n"
)

if (BUILD_PREMIUM)
    message("==> Build Premium <==")
    set(GR_PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
    set(GR_PROJECT_BINARY_PATH ${CMAKE_CURRENT_BINARY_DIR})
    set(WEBRTC_ROOT ${GR_PROJECT_PATH}/deps/tc_3rdparty/webrtc)
    add_definitions(-DPREMIUM_VERSION=1)

    configure_file(${CMAKE_SOURCE_DIR}/app_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/app_config.h)

    message("Webrtc Root: ${WEBRTC_ROOT}")
else()
    message("==> Build OpenSource <==")
    set(GR_PROJECT_PATH ${CMAKE_SOURCE_DIR})
    set(GR_PROJECT_BINARY_PATH ${CMAKE_BINARY_DIR})
    set(WEBRTC_ROOT ${GR_PROJECT_PATH}/deps/tc_3rdparty/webrtc)
    add_definitions(-DPREMIUM_VERSION=0)

    configure_file(${CMAKE_SOURCE_DIR}/app_config.h.in ${CMAKE_BINARY_DIR}/app_config.h)

    message("Webrtc Root: ${WEBRTC_ROOT}")
endif ()

if (JEMALLOC_ENABLED)
    add_definitions(-DJEMALLOC_ON=1)
endif ()

if (MEMORY_STST_ENABLED)
    add_definitions(-DMEMORY_STST_ON=1)
endif ()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

set(FFMPEG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg7.1.1)

set(LIBPLACEBO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_3rdparty/libplacebo)

#find_package(OpenSSL REQUIRED)
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts Svg)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Charts Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Charts Svg)

find_package(SDL2 CONFIG REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-libvpx CONFIG REQUIRED)
find_package(FFTW3 CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(cpr CONFIG REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/version_config.h)
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/icon.rc"
        @ONLY
)

include_directories(${GR_PROJECT_PATH}/)
include_directories(${GR_PROJECT_PATH}/src)
include_directories(${GR_PROJECT_PATH}/deps)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/eventbus/src)
include_directories(${GR_PROJECT_BINARY_PATH})
include_directories(${GR_PROJECT_BINARY_PATH}/deps/tc_message_new)
include_directories(${GR_PROJECT_BINARY_PATH}/deps/tc_server_protocol)
include_directories(${GR_PROJECT_BINARY_PATH}/deps/tc_3rdparty/qwindowkit/include)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/asio2/include)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/asio2/3rd)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/asio2/3rd/asio)
#include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/protobuf/src)
#include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/abseil-cpp)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/sqlite35)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/sqlite_orm/include)
#include_directories(${VCPKG_ROOT}/installed/x64-windows-static/include)
include_directories(${FFMPEG_ROOT}/include)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/webrtc/include/third_party/libyuv/include)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/leveldb/include)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/qwindowkit)
include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/qwindowkit/src)
include_directories(${GR_PROJECT_PATH}/src/render)
include_directories(${GR_PROJECT_PATH}/deps/tc_qt_widget)
include_directories(${GR_PROJECT_PATH}/deps/tc_client_sdk_new)
include_directories(${VCPKG_ROOT}/packages/breakpad_x64-windows-static/include)

link_directories(${VCPKG_ROOT}/installed/x64-windows/lib)
link_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/openssl/x64/lib/VC/static)

if(MSVC)
    add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
    add_compile_options("/bigobj")
    include_directories(${FFMPEG_ROOT}/include)
    link_directories(${FFMPEG_ROOT}/lib)
    link_directories(${LIBPLACEBO_ROOT}/lib)

    include_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/openssl/include)
    link_directories(${GR_PROJECT_PATH}/deps/tc_3rdparty/openssl/x64/lib/VC)
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:83886080")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,83886080")
endif()

add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
add_compile_definitions(ASIO2_ENABLE_SSL) #ASIO2_ENABLE_LOG
add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DENABLE_SSL=0 -D_AMD64_)#-DCMAKE_GENERATOR_PLATFORM=x64
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")

set(ENABLE_HOOK_CAPTURE ON)

set(Protobuf_SRC_ROOT_FOLDER ${GR_PROJECT_PATH}/deps/tc_3rdparty/protobuf)
if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    set(Protobuf_LIBRARIES ${GR_PROJECT_BINARY_PATH}/deps/tc_3rdparty/protobuf/cmake/libprotobuf.lib)
endif()
find_package(Protobuf REQUIRED)

set(OpenCV_ROOT ${GR_PROJECT_PATH}/deps/tc_3rdparty/opencv_410)
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

set(RES resources/main.qrc resources/qss.qrc res.qrc)
add_executable(${PROJECT_NAME} WIN32
        main.cpp
        ${RES}
        ${MAIN_SRCS}
        ${NETWORK_SRCS}
        ${CMAKE_CURRENT_BINARY_DIR}/icon.rc
        ${SERVICE_MGR_SRCS}
        ${DEVICE_LIST_SRCS}
)

#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS")

#### GUARD BEG ####
add_subdirectory(src/render_panel/guard)
message("===> Guard SRCS: ${GUARD_SRCS}")
add_executable(GammaRayGuard WIN32 ${GUARD_SRCS})
target_link_libraries(GammaRayGuard PRIVATE Qt6::Widgets Qt6::Core Qt6::Core5Compat tc_common_new winmm.lib SetupAPI.lib comsuppw.lib gflags::gflags)
SET_TARGET_PROPERTIES(GammaRayGuard PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\"")
#### GUARD END ####

file(COPY ${GR_PROJECT_PATH}/src/render/settings.toml DESTINATION ${GR_PROJECT_BINARY_PATH}/)
file(COPY ${GR_PROJECT_PATH}/certs DESTINATION ${GR_PROJECT_BINARY_PATH}/)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_client_web/" DESTINATION ${GR_PROJECT_BINARY_PATH}/www)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_message_new/" DESTINATION ${GR_PROJECT_BINARY_PATH}/www)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Network Qt6::Core Qt6::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg sqlite35
        tc_common_new tc_steam_manager_new tc_vigem tc_message tc_cp_message tc_plugin tc_qt_widget winmm.lib SetupAPI.lib gflags::gflags#tc_signaling_sdk #lib_GammaRayClient
        tc_relay_client tc_translator tc_spvr_client Dwmapi.lib tc_widget_loadings tc_client_plugin #QWindowKit::Widgets TC_WidgetFrame
        tc_account_sdk tc_rp_message tc_profile_client tc_hw_info tc_global_id_generator tc_server_message skin_interface skin_loader
)
set(BREAKPAD_LIB_PATH ${VCPKG_ROOT}/packages/breakpad_x64-windows-static/lib)
target_link_libraries(${PROJECT_NAME} PRIVATE ${BREAKPAD_LIB_PATH}/libbreakpad.lib ${BREAKPAD_LIB_PATH}/libbreakpad_client.lib)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${GR_PROJECT_BINARY_PATH}/gr_plugins
        COMMAND ${CMAKE_COMMAND} -E make_directory ${GR_PROJECT_BINARY_PATH}/gr_plugins
)

### Service Beg
### Service
add_executable(GammaRayService ${SERVICE_SRCS})
SET_TARGET_PROPERTIES(GammaRayService PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
target_link_libraries(GammaRayService PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(GammaRayService PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message tc_plugin tc_qt_widget sqlite35 winmm.lib SetupAPI.lib)

### Service Manager
add_executable(GammaRayServiceManager ${SERVICE_MGR_MAIN_SRCS} ${SERVICE_MGR_SRCS})
SET_TARGET_PROPERTIES(GammaRayServiceManager PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")

target_link_libraries(GammaRayServiceManager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(GammaRayServiceManager PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message tc_plugin tc_qt_widget sqlite35 winmm.lib SetupAPI.lib)
### Service End

### Uninstall
add_executable(uninstall WIN32
        icon.rc
        ${UNINSTALL_SRCS}
        ${SERVICE_MGR_SRCS})

target_link_libraries(uninstall PRIVATE gflags)

SET_TARGET_PROPERTIES(uninstall PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
add_executable(shadow_deleter ${SHADOW_DELETER_SRCS})
SET_TARGET_PROPERTIES(shadow_deleter PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup /MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")

target_link_libraries(uninstall PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network tc_qt_widget
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)
target_link_libraries(uninstall PRIVATE tc_common_new tc_steam_manager_new tc_vigem tc_message sqlite35 tc_plugin winmm.lib SetupAPI.lib)
### Uninstall End

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_target(tc_build_all ALL COMMENT "Build all targets")
add_dependencies(tc_build_all ${PROJECT_NAME} sqlite35 GammaRay GammaRayRender GammaRayService GammaRayServiceManager tc_message tc_vigem tc_plugin
        uninstall shadow_deleter GammaRayClientInner tc_relay_client tc_rtc_client tc_cp_message GammaRayGuard tc_hw_info GammaRayCrashReporter)
if(ENABLE_HOOK_CAPTURE)
    #add_dependencies(tc_build_all tc_graphics tc_graphics_helper tc_graphics_util) #tc_capture_library
endif ()
#

# gdi 
add_dependencies(tc_build_all plugin_net_ws plugin_ffmpeg_encoder plugin_frame_debugger plugin_dda_capture
    plugin_was_audio_capture plugin_opus_encoder tc_sdk tc_qt_widget plugin_mock_video_stream tc_translator
    tc_spvr_client tc_account_sdk tc_rp_message tc_profile_client plugin_event_replayer tc_global_id_generator
    tc_server_message skin_interface skin_loader skin_opensource
)

# client plugins
#add_dependencies(tc_build_all plugin_multi_screens plugin_media_record_client plugin_client_clipboard)

add_custom_target(tc_build_client ALL COMMENT "Build client")
add_dependencies(tc_build_client GammaRayClientInner)

add_custom_command(TARGET tc_build_all POST_BUILD
# gdi 
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_controller/vigem/driver/ViGEmBus_1.22.0_x64_x86_arm64.exe"  "${GR_PROJECT_BINARY_PATH}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/icon.ico"  "${GR_PROJECT_BINARY_PATH}/gr_icon.ico"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/client/icon.ico"  "${GR_PROJECT_BINARY_PATH}/gr_client_icon.ico"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/install.py"  "${GR_PROJECT_BINARY_PATH}/package/install.py"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GR_PROJECT_PATH}/package"  "${GR_PROJECT_BINARY_PATH}/package"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GR_PROJECT_PATH}/resources/qt_material"  "${GR_PROJECT_BINARY_PATH}/resources/qt_material"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GR_PROJECT_PATH}/resources/language"  "${GR_PROJECT_BINARY_PATH}/resources/language"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/settings.toml"  "${GR_PROJECT_BINARY_PATH}/"
    COMMAND ${CMAKE_COMMAND} -E copy "${OpenCV_ROOT}/x64/vc16/bin/opencv_world4100.dll"  "${GR_PROJECT_BINARY_PATH}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GR_PROJECT_PATH}/prebuild/win/gr_sysinfo.exe"  "${GR_PROJECT_BINARY_PATH}/"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/tc_rtc_client.dll"  "${GR_PROJECT_BINARY_PATH}/gr_client/tc_rtc_client.dll"

    ###### Renderer Plugins ######

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_net_ws.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_net_ws.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/net_ws/plugin_net_ws.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_net_ws.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_ffmpeg_encoder.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_ffmpeg_encoder.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/ffmpeg_encoder/plugin_ffmpeg_encoder.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_ffmpeg_encoder.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_frame_debugger.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_frame_debugger.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/frame_debugger/plugin_frame_debugger.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_frame_debugger.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_dda_capture.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_dda_capture.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/dda_capture/plugin_dda_capture.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_dda_capture.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_mock_video_stream.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_mock_video_stream.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/mock_video_stream/plugin_mock_video_stream.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_mock_video_stream.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_was_audio_capture.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_was_audio_capture.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/was_audio_capture/plugin_was_audio_capture.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_was_audio_capture.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_opus_encoder.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_opus_encoder.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/opus_encoder/plugin_opus_encoder.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_opus_encoder.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_event_replayer.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_event_replayer.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/render/plugins/event_replayer/plugin_event_replayer.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_event_replayer.dll.toml"

    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/render/plugins/frame_carrier/plugin_frame_carrier.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_frame_carrier.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/render/plugins/frame_carrier/plugin_frame_carrier.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins/plugin_frame_carrier.dll.toml"

    ###### Client Plugins ######
    # COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/plugin_multi_screens.dll"  "${GR_PROJECT_BINARY_PATH}/gr_plugins_client/plugin_multi_screens.dll"
    # COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/src/client/plugins/multi_screens/plugin_multi_screens.dll.toml"  "${GR_PROJECT_BINARY_PATH}/gr_plugins_client/plugin_multi_screens.dll.toml"

    ### Skin
    COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_BINARY_PATH}/skin_opensource.dll"  "${GR_PROJECT_BINARY_PATH}/gr_skins/skin_opensource.dll"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/src/skin/skin_config.toml" "${GR_PROJECT_BINARY_PATH}/gr_skins/skin_config.toml"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release
        COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_controller/vigem/driver/ViGEmBus_1.22.0_x64_x86_arm64.exe"  "${GR_PROJECT_BINARY_PATH}/"
)