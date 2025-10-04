cmake_minimum_required (VERSION 3.8)

project ("CMakeProject"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(MY_HEADERS_DIR ${PROJECT_SOURCE_DIR}/primitives/include)
set(MY_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/primitives)

include_directories(${MY_INSTALL_INCLUDE_DIR})
include_directories(${MY_HEADERS_DIR})

add_subdirectory (primitives)
add_subdirectory(tests)
