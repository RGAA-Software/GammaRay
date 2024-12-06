cmake_minimum_required(VERSION 3.0)

project(amf_encoder_shared VERSION 1.0.0)

add_definitions(-D_MBCS)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS /GL /W3 /Gy /Zc:wchar_t /Zi /Gm- /O2 /sdl /Zc:inline /fp:precise /errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD /FC /EHsc /nologo /diagnostics:classic ")

aux_source_directory(. SHARED_SRCS)

add_library(amf_encoder_shared ${SHARED_SRCS})