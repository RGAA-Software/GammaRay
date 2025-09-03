project(GammaRayClient)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message("Client => CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core OpenGL OpenGLWidgets Svg Charts)
find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/plugins)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/plugins/file_transfer_client)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/plugins/file_transfer_client/src)

qt_add_resources(RES ${GR_PROJECT_PATH}/resources/main.qrc resources.qrc)
add_compile_options(/bigobj)

set(SRCS_MAIN ct_main.cpp)

set(SRCS_WS
    ct_main_ws.cpp
    ct_client_context.cpp
    ct_workspace.cpp
    ct_base_workspace.cpp
    front_render/ct_video_widget_event.cpp
    front_render/opengl/ct_director.cpp
    front_render/opengl/ct_opengl_video_widget.cpp
    front_render/opengl/ct_shader_program.cpp
    front_render/opengl/ct_opengl_image_reader.cpp
    front_render/opengl/ct_renderer.cpp
    front_render/opengl/ct_sprite.cpp
    front_render/sdl/ct_sdl_video_widget.cpp
    ct_qt_key_converter.cpp
    ct_audio_player.cpp
    ct_main_progress.cpp
    ct_game_view.cpp
    icon.rc ${RES}
    ui/switch_button.cpp
    ui/float_icon.cpp
    ui/round_rect_widget.cpp
    ui/app_menu.cpp
    ui/app_color_theme.cpp
    ui/app_content.cpp
    ui/background_widget.cpp
    ct_settings.cpp
    ../theme/QtAdvancedStylesheet.cpp
    ui/float_controller.cpp
    ui/base_widget.cpp
    ui/float_controller_panel.cpp
    ui/float_notification_handle.cpp
    ui/notification_panel.cpp
    ui/notification_item.cpp
    ui/key_state_panel.cpp
    ui/ct_statistics_panel.cpp
    ui/ct_stat_chart.cpp
    ui/ct_stat_frame_info_item.cpp
    ui/float_sub_control_panel.cpp
    ui/float_sub_mode_panel.cpp
    ui/float_sub_fps_panel.cpp
    ui/float_sub_display_panel.cpp
    ui/float_3rd_scale_panel.cpp
    ui/float_3rd_resolution_panel.cpp
    ui/float_button_state_indicator.cpp
    ui/computer_icon.cpp
    ui/svg_lable.cpp
    ui/retry_conn_dialog.cpp
    network/ct_dashboard_client.cpp
    network/ct_panel_client.cpp
    plugins/ct_plugin_manager.cpp
    plugins/ct_plugin_event_router.cpp
)

if (WIN32)
    list(APPEND SRCS_WIN
        front_render/d3d11/ct_d3d11_video_widget.cpp
        front_render/d3d11/d3d11_render_manager.cpp
        front_render/d3d11/raw_sdl_widget.cpp
    )
endif ()

add_subdirectory(plugin_interface)
add_subdirectory(plugins)
add_executable(GammaRayClientInner ${SRCS_WS} ${CLIPBOARD_SRCS} ${CLIPBOARD_WIN_SRCS} ${SRCS_WIN}) #WIN32 MANUAL_FINALIZATION
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
#SET_TARGET_PROPERTIES(GammaRayClientInner PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
if (APPLE)
#    set_target_properties(${PROJECT_NAME} PROPERTIES
#            MACOSX_BUNDLE_GUI_IDENTIFIER com.tc.client.client
#            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#            MACOSX_BUNDLE TRUE
#            WIN32_EXECUTABLE TRUE
#    )
    set_target_properties(GammaRayClientInner PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER com.tc.client.client
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(GammaRayClientInner PRIVATE Qt6::Widgets Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg Qt6::Charts
        tc_common_new tc_message tc_cp_message tc_sdk tc_qt_widget OpenSSL::SSL OpenSSL::Crypto #libssl64MT.lib libcrypto64MT.lib sqlite_orm::sqlite_orm
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static> Winmm.lib #QWindowKit::Widgets TC_WidgetFrame
        tc_rtc_client Dwmapi.lib tc_client_plugin sqlite35
)

if(QT_VERSION_MAJOR EQUAL 6)
#    qt_finalize_executable(${PROJECT_NAME})
#    qt_finalize_executable(${PROJECT_NAME}Inner)
endif()

if (WIN32)
    add_custom_command(TARGET GammaRayClientInner POST_BUILD
            COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:GammaRayClientInner> --no-translations --release
            #COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/qt_material"  "${CMAKE_CURRENT_BINARY_DIR}/resources/qt_material"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/avcodec-59.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/avdevice-59.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/avfilter-8.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/avutil-57.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/avformat-59.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/postproc-56.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/swresample-4.dll"  "${GR_PROJECT_BINARY_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${GR_PROJECT_PATH}/deps/tc_3rdparty/ffmpeg/lib/x64/swscale-6.dll"  "${GR_PROJECT_BINARY_PATH}"
    )

elseif (APPLE)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${QT_ROOT}/bin/macdeployqt $<TARGET_FILE:${PROJECT_NAME}> --no-translations
#    )
elseif (UNIX)

endif ()