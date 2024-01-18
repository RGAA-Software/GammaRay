cmake_minimum_required(VERSION 3.26)
include(env_settings.cmake)
set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(tc_server_steam VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Qt6 6.5 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Quick QuickControls2)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()
find_package(fmt CONFIG REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)

include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
link_directories(${VCPKG_ROOT}/installed/x64-windows/lib)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_subdirectory(deps)
add_subdirectory(src)

#qt_add_executable(${PROJECT_NAME}
add_executable(${PROJECT_NAME}
        main.cpp
        res.qrc
        ${MAIN_SRCS}
)

qt_add_resources(${PROJECT_NAME} "assets"
    PREFIX "/"
    FILES
        "resources/tc_icon.png"
)

qt_add_qml_module(${PROJECT_NAME}
        URI tc_server_steam
        VERSION 1.0
        QML_FILES
            Main.qml
            Test.qml
            TCButton.qml
            TCTabView.qml
            TCGameListView.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(${PROJECT_NAME} PROPERTIES
        #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.apptest_qml
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Qt6::Quick Qt6::Core Qt6::Qml Qt6::QuickControls2
)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}>
#        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources"  "${CMAKE_BINARY_DIR}/resources"
#        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/settings.toml"  "${CMAKE_BINARY_DIR}"
)
