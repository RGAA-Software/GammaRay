add_subdirectory(render_panel)

set(MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/app_colors.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/theme/QtAdvancedStylesheet.cpp
        ${PANEL_SRCS}
        PARENT_SCOPE
)

add_subdirectory(render)
