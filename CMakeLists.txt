cmake_minimum_required(VERSION 3.26)
include(env_settings.cmake)
set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(tc_server_steam VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()
#find_package(fmt CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_path(UWEBSOCKETS_INCLUDE_DIRS "uwebsockets/App.h")
find_package(SqliteOrm CONFIG REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)

include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/eventbus/src)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
link_directories(${VCPKG_ROOT}/installed/x64-windows/lib)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)
if(MSVC)
    add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
    add_compile_options("/bigobj")
endif()
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:83886080")
    message("---MSVC--set StackSize")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,83886080")
    message("---CLANG--set StackSize")
endif()
add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(src/tests)

#qt_add_executable(${PROJECT_NAME}
set(RES resources/font.qrc resources/main.qrc resources/qss.qrc res.qrc)
add_executable(${PROJECT_NAME}
        main.cpp
        ${RES}
        ${MAIN_SRCS}
        ${NETWORK_SRCS}
        icon.rc
)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")

#qt_add_resources(${PROJECT_NAME} "assets"
#    PREFIX "/"
#    FILES
#        "resources/tc_icon.png"
#        "resources/tc_icon_raw.png"
#)

#qt_add_qml_module(${PROJECT_NAME}
#        URI tc_server_steam
#        VERSION 1.0
#        QML_FILES
#            Main.qml
#            Test.qml
#            TCButton.qml
#            TCTabView.qml
#            TCGameListView.qml
#)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/certificate.pem" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/private.key" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_client_web/" DESTINATION ${CMAKE_BINARY_DIR}/www)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_message_new/" DESTINATION ${CMAKE_BINARY_DIR}/www)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
#set_target_properties(${PROJECT_NAME} PROPERTIES
#        #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest_qml
#        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#        MACOSX_BUNDLE TRUE
#        WIN32_EXECUTABLE TRUE
#)

#fmt::fmt-header-only
target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto tc_common_new tc_steam_manager_new sqlite_orm::sqlite_orm tc_vigem)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release
#        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources"  "${CMAKE_BINARY_DIR}/resources"
#        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/settings.toml"  "${CMAKE_BINARY_DIR}"
)
