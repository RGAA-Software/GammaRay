add_subdirectory(render_panel)

set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/app_colors.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_workspace.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_settings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_statistics.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_system_monitor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_resources.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_render_controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_run_game_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_base.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_settings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_general.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_about_me.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/game_info_preview.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/rn_app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/rn_empty.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/stat_chart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/qt_circle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/qt_vertical.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/add_game_panel.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/util/qt_directory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/util/dxgi_mon_detector.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/theme/QtAdvancedStylesheet.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/transfer/file_transfer.cpp

        ${PANEL_SRCS}

        PARENT_SCOPE
)

add_subdirectory(render)
