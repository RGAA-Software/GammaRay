cmake_minimum_required(VERSION 3.26)

project(GammaRayCrashReporter VERSION ${TC_APP_VERSION} LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    crash_reporter_main.cpp
)