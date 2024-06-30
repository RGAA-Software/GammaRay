find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Core5Compat)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Core5Compat)

if (WIN32)
    add_library(app_manager
        app_manager.cpp
        app_manager_factory.cpp
        app_timer.cpp
        app_shared_info.cpp
        steam_game.cpp

        win/app_manager_win.cpp
        win/dx_address_loader.cpp
        win/win_event_replayer.cpp
    )
    target_link_libraries(app_manager fftw3)
    target_link_libraries(app_manager Qt6::Core Qt6::Core5Compat)
elseif (APPLE)

elseif (UNIX)

endif ()