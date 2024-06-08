set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/workspace.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_settings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gr_statistics.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/system_monitor.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ws_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/udp_broadcaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ip_util.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/manager/gr_server_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/manager/run_game_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qrcodegen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qrcodegen.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qr_generator.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/app_skin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/custom_tab_btn.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/round_img_display.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/cover_widget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main_item_delegate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/sized_msg_box.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_base.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_settings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_general.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_client.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/widget_decorator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/rn_app.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/rn_empty.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/stat_chart.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/qt_circle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/qt_vertical.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/game_info_preview.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/db/db_game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/db/db_game_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/res/resource_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/util/qt_directory.cpp
        PARENT_SCOPE
)

add_subdirectory(server)
