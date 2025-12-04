cmake_minimum_required(VERSION 3.20.0)

# Set QT_ROOT in advance. If QT_ROOT is not set, the value in qt.cmake will be used

if(NOT DEFINED QT_ROOT)
	include(qt.cmake)
	set(CMAKE_PREFIX_PATH ${QT_ROOT}/lib/cmake)
endif()

project(gd_gui VERSION 1.0.1)

include(InstallRequiredSystemLibraries)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_VERBOSE_MAKEFILE ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
endif()

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Svg REQUIRED)

if(${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
	set(PLATFORM_WIN64 FALSE)
else()
	set(PLATFORM_WIN64 TRUE)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(UILIB_SOURCES
	src/gd_icon_button.cpp 
	src/gd_progress_bar.cpp 
	src/gd_button.cpp
	src/gd_line_edit.cpp
	src/gd_label.cpp
	src/gd_verification.cpp
	src/gd_menu.cpp
	src/gd_custom_progress_bar.cpp
)

include_directories(		
	${CMAKE_BINARY_DIR}
)


add_library(${PROJECT_NAME} STATIC ${UILIB_SOURCES})

target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Core Qt6::Svg)

add_subdirectory(exmaple)


