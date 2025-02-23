cmake_minimum_required(VERSION 3.8)
project(my_tests)

#add_subdirectory(lib/googletest)

include_directories(${MY_HEADERS_DIR})
#include_directories(lib/googletest/googletest/include)

set(SOURCE_FILES test.cpp)

add_executable(my_tests ${SOURCE_FILES})
target_compile_options(my_tests  
PRIVATE ${CFLAGS} )

install(TARGETS my_tests DESTINATION bin)

add_test(TestName my_test)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS my_test)
