set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/workspace.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/settings.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ws_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/udp_broadcaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ip_util.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/tc_app_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qrcodegen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qrcodegen.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qr_generator.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/app_skin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/custom_tab_btn.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/round_img_display.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/cover_widget.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets/main_item_delegate.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_base.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/tab_settings.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_input.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_media.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_network.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/st_encoder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/widget_decorator.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/db/game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/db/game_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/res/resource_manager.cpp

        PARENT_SCOPE
)
