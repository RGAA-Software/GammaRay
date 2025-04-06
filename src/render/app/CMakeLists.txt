find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Core5Compat Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Core5Compat Widgets)

if (WIN32)
    add_library(app_manager
        app_manager.cpp
        app_manager_factory.cpp
        app_timer.cpp
        app_shared_info.cpp
        steam_game.cpp
        clipboard_manager.cpp
        win/app_manager_win.cpp
        win/dx_address_loader.cpp
        win/win_event_replayer.cpp
        win/win_message_window.cpp
        win/win_desktop_manager.cpp
        win/win_message_loop.cpp
        video_frame_carrier.cpp
        monitor_refresher.cpp
    )
    target_link_libraries(app_manager tc_message frame_render libprotobuf)
    target_link_libraries(app_manager Qt6::Core Qt6::Core5Compat Qt6::Widgets)
elseif (APPLE)

elseif (UNIX)

endif ()