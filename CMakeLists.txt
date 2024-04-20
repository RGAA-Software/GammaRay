cmake_minimum_required(VERSION 3.26)
include(env_settings.cmake)
set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)
set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(GammaRay VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Core Core5Compat WebSockets Network Charts)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()

find_package(OpenSSL REQUIRED)
find_package(SqliteOrm CONFIG REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)

include_directories(${VK_SDK})
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/deps)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/eventbus/src)
include_directories(${CMAKE_BINARY_DIR}/deps/tc_message_new)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/include)
include_directories(${CMAKE_SOURCE_DIR}/deps/tc_3rdparty/asio2/3rd)
include_directories(${VCPKG_ROOT}/installed/x64-windows/include)

link_directories(${VCPKG_ROOT}/installed/x64-windows/lib)
link_directories(${VCPKG_ROOT}/packages/poco_x64-windows-static/lib)

if(MSVC)
    add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
    add_compile_options("/bigobj")
endif()

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:83886080")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--stack,83886080")
endif()

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -DENABLE_SSL=0)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

## Boost
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()

find_package(OpenSSL REQUIRED)
find_package(SqliteOrm CONFIG REQUIRED)
find_package(gflags CONFIG REQUIRED)
find_path(DETOURS_INCLUDE_DIRS "detours/detours.h")

### CPY
file(COPY ${CMAKE_SOURCE_DIR}/src/server/settings.toml DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/src/server/private.key DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/src/server/certificate.pem DESTINATION ${CMAKE_BINARY_DIR})
### CPY

add_subdirectory(deps)
add_subdirectory(src)
add_subdirectory(src/tests)

set(RES resources/font.qrc resources/main.qrc resources/qss.qrc res.qrc)
add_executable(${PROJECT_NAME}
        main.cpp
        ${RES}
        ${MAIN_SRCS}
        ${NETWORK_SRCS}
        icon.rc
)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level=\'requireAdministrator\' uiAccess=\'false\'\"")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/certificate.pem" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/private.key" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_client_web/" DESTINATION ${CMAKE_BINARY_DIR}/www)
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/deps/tc_message_new/" DESTINATION ${CMAKE_BINARY_DIR}/www)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Core5Compat Qt6::WebSockets Qt6::Network Qt6::Charts)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto tc_common_new tc_steam_manager_new sqlite_orm::sqlite_orm tc_vigem tc_message)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_target(tc_build_all COMMENT "Build all targets")
add_dependencies(tc_build_all ${PROJECT_NAME} GammaRay GammaRayServer tc_capture_library tc_graphics tc_graphics_util
        tc_message tc_graphics_helper tc_vigem)

add_custom_command(TARGET tc_build_all POST_BUILD
        COMMAND ${QT_ROOT}/bin/windeployqt.exe $<TARGET_FILE:${PROJECT_NAME}> --release
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/deps/tc_controller/vigem/driver/ViGEmBus_1.22.0_x64_x86_arm64.exe"  "${CMAKE_BINARY_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/src/server/settings.toml"  "${CMAKE_BINARY_DIR}"
        #        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/resources"  "${CMAKE_BINARY_DIR}/resources"
)
