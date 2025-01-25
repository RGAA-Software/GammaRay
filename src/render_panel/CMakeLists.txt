set(PANEL_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ws_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/udp_broadcaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ip_util.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/gr_service_client.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/db/db_game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/db/db_game_manager.cpp

        PARENT_SCOPE
)