if (WIN32)
    add_library(app_manager
        app_manager.cpp
        app_manager_factory.cpp
        app_timer.cpp
        app_shared_info.cpp
        steam_game.cpp

        #
        win/app_manager_win.cpp
        win/dx_address_loader.cpp
        win/control_event_replayer_win.cpp
    )
elseif (APPLE)

elseif (UNIX)

endif ()