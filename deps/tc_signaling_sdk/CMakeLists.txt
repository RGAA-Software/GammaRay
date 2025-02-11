project(tc_signaling_sdk)

add_library(${PROJECT_NAME}
        sig_sdk_ws_processor.cpp
        sig_sdk_maker.cpp
        sig_sdk_base_processor.cc
        sig_sdk_ws_router.cpp
        sig_sdk_context.cpp
)

