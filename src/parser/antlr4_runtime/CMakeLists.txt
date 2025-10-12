set(antlr4-runtime_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/antlr4-cpp-runtime/src)

set(libantlrcpp_INCLUDE_DIRS
  ${antlr4-runtime_DIR}
  ${antlr4-runtime_DIR}/atn
  ${antlr4-runtime_DIR}/dfa
  ${antlr4-runtime_DIR}/internal
  ${antlr4-runtime_DIR}/misc
  ${antlr4-runtime_DIR}/support
  ${antlr4-runtime_DIR}/tree
  ${antlr4-runtime_DIR}/tree/pattern
  ${antlr4-runtime_DIR}/tree/xpath
)

file(GLOB libantlrcpp_SRC
  "${antlr4-runtime_DIR}/*.cpp"
  "${antlr4-runtime_DIR}/atn/*.cpp"
  "${antlr4-runtime_DIR}/dfa/*.cpp"
  "${antlr4-runtime_DIR}/internal/*.cpp"
  "${antlr4-runtime_DIR}/misc/*.cpp"
  "${antlr4-runtime_DIR}/support/*.cpp"
  "${antlr4-runtime_DIR}/tree/*.cpp"
  "${antlr4-runtime_DIR}/tree/pattern/*.cpp"
  "${antlr4-runtime_DIR}/tree/xpath/*.cpp"
)

add_library(antlr4_static STATIC ${libantlrcpp_SRC})
target_include_directories(antlr4_static PUBLIC
  "$<BUILD_INTERFACE:${libantlrcpp_INCLUDE_DIRS}>"
  "$<INSTALL_INTERFACE:${libantlrcpp_INCLUDE_INSTALL_DIR}>")
target_compile_definitions(antlr4_static PUBLIC ANTLR4CPP_STATIC)

add_library(antlr4_component INTERFACE)
target_sources(antlr4_component
  INTERFACE
  ${libantlrcpp_SRC}
)
target_include_directories(antlr4_component INTERFACE
  "$<BUILD_INTERFACE:${libantlrcpp_INCLUDE_DIRS}>"
  "$<INSTALL_INTERFACE:${libantlrcpp_INCLUDE_INSTALL_DIR}>")
target_compile_definitions(antlr4_component INTERFACE ANTLR4CPP_STATIC)
