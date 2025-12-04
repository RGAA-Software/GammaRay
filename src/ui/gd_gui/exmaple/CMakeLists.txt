cmake_minimum_required(VERSION 3.20.0)

project(exmaple VERSION 1.0.1)

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

add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN)

if(${CMAKE_SIZEOF_VOID_P} STREQUAL "4")
	set(PLATFORM_WIN64 FALSE)
else()
	set(PLATFORM_WIN64 TRUE)
endif()

include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/src
)

set(EXAMPLE_SOURCES
	main.cpp
)

include_directories(		
	${CMAKE_BINARY_DIR}
)

link_directories(
)

if(${GENERATE_SETUP})
	#WIN32
	add_executable(${PROJECT_NAME} #WIN32
		${EXAMPLE_SOURCES}
	)
else()
	add_executable(${PROJECT_NAME}
		${EXAMPLE_SOURCES}
	)
endif()

target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Core Qt6::Svg gd_gui)

message(QT_ROOT = ${QT_ROOT})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_custom_command(
					TARGET ${PROJECT_NAME}
					POST_BUILD
					COMMAND ${QT_ROOT}/bin/windeployqt.exe --debug ${PROJECT_NAME}.exe
					WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
else()
	add_custom_command(
					TARGET ${PROJECT_NAME}
					POST_BUILD
					COMMAND ${QT_ROOT}/bin/windeployqt.exe --release ${PROJECT_NAME}.exe
					WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
	)
endif()
