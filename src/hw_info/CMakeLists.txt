project(tc_hw_info)
add_library(${PROJECT_NAME}
    hw_info_parser.cpp
    hw_info_widget.cpp
    hw_stat_chart.cpp
    hw_cpu_detail_widget.cpp
    hw_cpu_detail_item.cpp
)
target_link_libraries(${PROJECT_NAME} tc_common_new)
target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Network Qt6::Core Qt6::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts Qt6::Svg)