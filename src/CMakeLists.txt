set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/entity/game.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/model/game_model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/steam/steam_manager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/application.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/context.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/network/app_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/network/http_server.cpp

        PARENT_SCOPE
)

# add_subdirectory(network)
