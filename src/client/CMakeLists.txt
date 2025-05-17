project(GammaRayClient)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message("Client ==> CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("Client ==> BUILD_FROM: ${BUILD_FROM}")
set(CLIENT_BUILD_TYPE_PATH "src/")
if(${BUILD_FROM} STREQUAL "CMD")
    set(CLIENT_BUILD_TYPE_PATH "${CMAKE_BUILD_TYPE}/")
else ()

endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Core OpenGL OpenGLWidgets Svg Charts)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/client)

qt_add_resources(RES ${CMAKE_SOURCE_DIR}/resources/font.qrc ${CMAKE_SOURCE_DIR}/resources/main.qrc resources.qrc)
add_compile_options(/bigobj)

set(SRCS_MAIN ct_main.cpp)

set(SRCS_WS
        ct_main_ws.cpp
        ct_client_context.cpp
        ct_workspace.cpp
        ct_director.cpp
        ct_video_widget_event.cpp
        ct_shader_program.cpp
        ct_opengl_image_reader.cpp
        ct_renderer.cpp
        ct_sprite.cpp
        ct_opengl_video_widget.cpp
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
        ui/float_sub_display_panel.cpp
        ui/float_3rd_scale_panel.cpp
        ui/float_3rd_resolution_panel.cpp
        ui/float_button_state_indicator.cpp
        ui/computer_icon.cpp
        transfer/file_transfer.cpp
        transfer/fs_object.cpp
        transfer/file_writer.cpp
        transfer/fs_file.cpp
        transfer/fs_folder.cpp
        network/ct_dashboard_client.cpp
        network/ct_panel_client.cpp
)

if(TC_USE_FILE_TRANSMISSION)
    message(STATUS "file transmission is enabled.")
    add_subdirectory(./file_transmission_client/src)
    include_directories(./file_transmission_client/src)
    add_definitions(-DTC_ENABLE_FILE_TRANSMISSION)
else()
    message(STATUS "file transmission is disabled.")
endif()

add_subdirectory(clipboard)
add_executable(GammaRayClientInner ${SRCS_WS} ${CLIPBOARD_SRCS} ${CLIPBOARD_WIN_SRCS}) #WIN32 MANUAL_FINALIZATION
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
        tc_common_new tc_message tc_cp_message tc_sdk tc_qt_widget sqlite_orm::sqlite_orm libssl64MT.lib libcrypto64MT.lib
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static> QWindowKit::Widgets TC_WidgetFrame Winmm.lib
        tc_rtc_client
)

if(TC_USE_FILE_TRANSMISSION) #file_transfer
    target_link_libraries(GammaRayClientInner PRIVATE file_transfer
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
#    qt_finalize_executable(${PROJECT_NAME})
#    qt_finalize_executable(${PROJECT_NAME}Inner)
endif()

if (WIN32)
    add_custom_command(TARGET GammaRayClientInner POST_BUILD
            COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:GammaRayClientInner> --no-translations --release
            #COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/qt_material"  "${CMAKE_CURRENT_BINARY_DIR}/resources/qt_material"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avcodec-59.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avdevice-59.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avfilter-8.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avutil-57.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avformat-59.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/postproc-56.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/swresample-4.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/swscale-6.dll"  "${CMAKE_BINARY_DIR}/${CLIENT_BUILD_TYPE_PATH}"
    )

elseif (APPLE)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${QT_ROOT}/bin/macdeployqt $<TARGET_FILE:${PROJECT_NAME}> --no-translations
#    )
elseif (UNIX)

endif ()