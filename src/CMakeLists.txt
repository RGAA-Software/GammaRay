add_subdirectory(hw_info)
add_subdirectory(render_panel)
add_subdirectory(render_panel/system)

set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/app_colors.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/theme/QtAdvancedStylesheet.cpp
        ${PANEL_SRCS}
        ${SYSTEM_SRCS}
        PARENT_SCOPE
)

add_subdirectory(render)
add_subdirectory(skin)
