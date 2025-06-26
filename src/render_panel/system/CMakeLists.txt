if (WIN32)
    set(SYSTEM_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/win/win_panel_message_loop.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/win/win_panel_message_window.cpp
            PARENT_SCOPE
    )
else ()

endif ()