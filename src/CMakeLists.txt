set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/entity/game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/game_model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/steam/steam_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/context.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ws_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/udp_broadcaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/ip_util.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/app/tc_app_manager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qrcodegen.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qrcodegen.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/qrcode/qr_generator.cpp

        PARENT_SCOPE
)

# add_subdirectory(network)
