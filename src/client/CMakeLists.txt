project(GammaRayClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core OpenGL OpenGLWidgets Svg)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/client)

link_directories(${VCPKG_ROOT}/installed/x64-windows-static/lib)

set(RES ${CMAKE_SOURCE_DIR}/resources/font.qrc ${CMAKE_SOURCE_DIR}/resources/main.qrc ${CMAKE_SOURCE_DIR}/resources/qss.qrc resources.qrc)
add_compile_options(/bigobj)
set(SRCS
        main.cpp
        client_context.cpp
        icon.rc ${RES}
        ui/switch_button.cpp
        ui/float_button.cpp
        ui/message_dialog.cpp
        ui/round_rect_widget.cpp
        ui/app_menu.cpp
        ui/app_stream_list.cpp
        ui/create_stream_dialog.cpp
        ui/app_color_theme.cpp
        ui/app_content.cpp
        ui/stream_item_widget.cpp
        ui/stream_content.cpp
        ui/settings_content.cpp
        ui/about_content.cpp
        ui/sized_msg_box.cpp
        ui/background_widget.cpp
        ui/base_widget.cpp
        ui/multi_display_mode_widget.cpp
        db/stream_db_manager.cpp
        db/stream_item.cpp
        application.cpp
        settings.cpp
        ../theme/QtAdvancedStylesheet.cpp
)

set(SRCS_WS
        main_ws.cpp
        client_context.cpp
        workspace.cpp
        director.cpp
        video_widget_event.cpp
        shader_program.cpp
        opengl_image_reader.cpp
        renderer.cpp
        sprite.cpp
        opengl_video_widget.cpp
        qt_key_converter.cpp
        audio_player.cpp
        clipboard_manager.cpp
        icon.rc ${RES}
        ui/switch_button.cpp
        ui/float_button.cpp
        ui/float_icon.cpp
        ui/message_dialog.cpp
        ui/round_rect_widget.cpp
        ui/app_menu.cpp
        ui/app_color_theme.cpp
        ui/app_content.cpp
        ui/sized_msg_box.cpp
        ui/background_widget.cpp
        db/stream_db_manager.cpp
        db/stream_item.cpp
        settings.cpp
        ../theme/QtAdvancedStylesheet.cpp
        ui/float_controller.cpp
        ui/base_widget.cpp
        ui/float_controller_panel.cpp
        ui/float_notification_handle.cpp
        ui/notification_panel.cpp
        ui/notification_item.cpp
        ui/key_state_panel.cpp
        ui/debug_panel.cpp
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
)


qt_add_executable(${PROJECT_NAME} WIN32 MANUAL_FINALIZATION ${SRCS})
qt_add_executable(GammaRayClientInner  MANUAL_FINALIZATION ${SRCS_WS}) #WIN32
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
#SET_TARGET_PROPERTIES(GammaRayClientInner PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")
if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER com.tc.client.client
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
    )
    set_target_properties(GammaRayClientInner PROPERTIES
            MACOSX_BUNDLE_GUI_IDENTIFIER com.tc.client.client
            MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
            MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
            MACOSX_BUNDLE TRUE
            WIN32_EXECUTABLE TRUE
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets
        Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
)
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_message tc_sdk tc_qt_widget)
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite_orm::sqlite_orm ${Boost_LIBRARIES})

target_link_libraries(GammaRayClientInner PRIVATE Qt6::Widgets
        Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
)
target_link_libraries(GammaRayClientInner PRIVATE tc_common_new tc_message tc_sdk tc_qt_widget)
target_link_libraries(GammaRayClientInner PRIVATE sqlite_orm::sqlite_orm ${Boost_LIBRARIES} libssl64MT.lib libcrypto64MT.lib)
target_link_libraries(GammaRayClientInner
        PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
    qt_finalize_executable(${PROJECT_NAME}Inner)
endif()

if (WIN32)
    add_custom_command(TARGET GammaRayClientInner POST_BUILD
            COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:GammaRayClientInner> --no-translations --release
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/resources/qt_material"  "${CMAKE_CURRENT_BINARY_DIR}/resources/qt_material"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avcodec-59.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avdevice-59.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avfilter-8.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avutil-57.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/avformat-59.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/postproc-56.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/swresample-4.dll"  "${CMAKE_BINARY_DIR}/src"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/ffmpeg/lib/x64/swscale-6.dll"  "${CMAKE_BINARY_DIR}/src"
    )

elseif (APPLE)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${QT_ROOT}/bin/macdeployqt $<TARGET_FILE:${PROJECT_NAME}> --no-translations
#    )
elseif (UNIX)

endif ()