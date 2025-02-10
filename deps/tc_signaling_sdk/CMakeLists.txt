project(tc_signaling_sdk)

include_directories(${CMAKE_SOURCE_DIR}/tc_3rdparty/asio2/include)
include_directories(${CMAKE_SOURCE_DIR}/tc_3rdparty/asio2/3rd)

add_library(${PROJECT_NAME}
        #sig_wss_manager.cc
        sig_ws_manager.cc
        sig_maker.cc
        sig_mgr_interface.cc
        #sig_router_wss_impl.cc
        sig_router_ws_impl.cc
		rtc_context.cc
        )

target_compile_options(${PROJECT_NAME}
        PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
        )

