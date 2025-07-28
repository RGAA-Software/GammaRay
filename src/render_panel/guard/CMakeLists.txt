if (WIN32)
    set(GUARD_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/gr_guard_main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/gr_panel_guard.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/gr_guard_context.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/gr_guard_app.cpp
            PARENT_SCOPE
    )
else ()

endif ()