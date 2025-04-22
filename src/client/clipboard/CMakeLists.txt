set(CLIPBOARD_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/ct_clipboard_manager.cpp
    PARENT_SCOPE
)

if(WIN32)
    set(CLIPBOARD_WIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/win/cp_data_object.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/cp_file_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/cp_virtual_file.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/win_message_loop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/win/win_message_window.cpp

        PARENT_SCOPE
    )
else ()
    set(CLIPBOARD_WIN_SRCS)
endif ()