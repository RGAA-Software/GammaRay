project(plugin_clipboard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PLUGIN_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/clipboard_plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/clipboard_manager.cpp
)

if (WIN32)
    set(CLIPBOARD_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/win/DataObject.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/win/VirtualFIleSrcStream.cpp
    )
else ()

endif ()

add_library(${PROJECT_NAME} SHARED ${PLUGIN_SRCS} ${CLIPBOARD_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE tc_common_new tc_message tc_plugin tc_opus_codec_new Winmm.lib)