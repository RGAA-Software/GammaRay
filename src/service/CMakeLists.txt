set(SERVICE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/service_main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/service_msg_server.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/render_manager.cpp
        PARENT_SCOPE
)

set(SERVICE_MGR_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/service_manager.cpp
        PARENT_SCOPE
)

set(SERVICE_MGR_MAIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/service_manager_main.cpp
        PARENT_SCOPE
)