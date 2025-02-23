cmake_minimum_required (VERSION 3.8)
project(primitives CXX)

set(SOURCE_FILES
  mcsSpinlock.hpp
  github_mcs.hpp
)

add_library(primitives SHARED STATIC ${SOURCE_FILES})

install(TARGETS primitives DESTINATION ${MY_INSTALL_LIB_DIR})
install(FILES mcsSpinlock.hpp DESTINATION ${MY_INSTALL_INCLUDE_DIR})
install(FILES github_mcs.hpp DESTINATION ${MY_INSTALL_INCLUDE_DIR})

set_target_properties(primitives PROPERTIES LINKER_LANGUAGE CXX)
