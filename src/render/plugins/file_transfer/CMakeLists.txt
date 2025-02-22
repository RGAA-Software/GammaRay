if(${TC_USE_FILE_TRANSMISSION})
    message(STATUS "file transmission(server) is enabled.")
    project(plugin_file_transfer)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/file_transfer_plugin.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/file_transmission_server/file_operate.cc
        #${CMAKE_CURRENT_SOURCE_DIR}/file_transmission_server/file_transmit_impl.cc
        #${CMAKE_CURRENT_SOURCE_DIR}/file_transmission_server/file_transmit_manager.cc
    )

    add_compile_options("/bigobj")
    if(MSVC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:83886080")
    endif()

    add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core tc_common_new tc_net_plugin libprotobuf tc_message)
endif()

