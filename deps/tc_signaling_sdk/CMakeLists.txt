project(tc_signaling_sdk)

add_library(${PROJECT_NAME}
        sig_sdk_ws_manager.cpp
        sig_sdk_maker.cpp
        sig_sdk_mgr_interface.cc
        sig_sdk_router_ws_impl.cpp
        sig_sdk_context.cpp
)

