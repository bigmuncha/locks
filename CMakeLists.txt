cmake_minimum_required (VERSION 3.8)

project ("CMakeProject"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CFLAGS "-O2")
list(APPEND CFLAGS "-g")
#list(APPEND CFLAGS "-fsanitize=undefined")
#SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined") 
#set(MY_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(MY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/primitives/include)
set(MY_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/primitives)

include_directories(${MY_INSTALL_INCLUDE_DIR})
include_directories(${MY_HEADERS_DIR})

add_subdirectory (primitives)
add_subdirectory(tests)
