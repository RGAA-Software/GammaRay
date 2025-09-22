project(tc_hw_info)
add_library(${PROJECT_NAME}
    hw_info_parser.cpp
)
target_link_libraries(${PROJECT_NAME} tc_common_new)
target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Network Qt6::Core Qt6::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)