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

find_package(Qt6 REQUIRED COMPONENTS Widgets Core OpenGL OpenGLWidgets Svg)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/client)

#link_directories(${VCPKG_ROOT}/installed/x64-windows-static/lib)

qt_add_resources(RES ${CMAKE_SOURCE_DIR}/resources/font.qrc ${CMAKE_SOURCE_DIR}/resources/main.qrc resources.qrc)
add_compile_options(/bigobj)

set(SRCS_MAIN ct_main.cpp)

set(SRCS
        ct_client_context.cpp
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
        ui/background_widget.cpp
        ui/base_widget.cpp
        ui/multi_display_mode_widget.cpp
        db/stream_db_manager.cpp
        db/stream_item.cpp
        ct_application.cpp
        ct_settings.cpp
        ../theme/QtAdvancedStylesheet.cpp
)

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
        ct_clipboard_manager.cpp
        ct_main_progress.cpp
        icon.rc ${RES}
        ui/switch_button.cpp
        ui/float_button.cpp
        ui/float_icon.cpp
        ui/message_dialog.cpp
        ui/round_rect_widget.cpp
        ui/app_menu.cpp
        ui/app_color_theme.cpp
        ui/app_content.cpp
        ui/background_widget.cpp
        db/stream_db_manager.cpp
        db/stream_item.cpp
        ct_settings.cpp
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

if(TC_USE_FILE_TRANSMISSION)
    message(STATUS "file transmission is enabled.")
    add_subdirectory(./file_transmission_client/src)
    include_directories(./file_transmission_client/src)
    add_definitions(-DTC_ENABLE_FILE_TRANSMISSION)
else()
    message(STATUS "file transmission is disabled.")
endif()

add_library(lib_${PROJECT_NAME} ${SRCS})
target_link_libraries(lib_${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
        tc_common_new tc_message tc_sdk tc_qt_widget sqlite_orm::sqlite_orm
)

qt_add_executable(${PROJECT_NAME} WIN32 MANUAL_FINALIZATION ${SRCS} ${SRCS_MAIN})
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

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
        tc_common_new tc_message tc_sdk tc_qt_widget sqlite_orm::sqlite_orm QWindowKit::Widgets TC_WidgetFrame
)

target_link_libraries(GammaRayClientInner PRIVATE Qt6::Widgets Qt6::Core Qt6::OpenGL Qt6::OpenGLWidgets Qt6::Svg
        tc_common_new tc_message tc_sdk tc_qt_widget sqlite_orm::sqlite_orm libssl64MT.lib libcrypto64MT.lib
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static> QWindowKit::Widgets TC_WidgetFrame
)

if(TC_USE_FILE_TRANSMISSION) #file_transfer
    target_link_libraries(GammaRayClientInner PRIVATE file_transfer
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
    qt_finalize_executable(${PROJECT_NAME}Inner)
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