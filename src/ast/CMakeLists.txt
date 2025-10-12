# This library is just to provide include directory for other libraries.
add_library(gql_ast INTERFACE)

target_include_directories(gql_ast
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

add_library(gql_ast_print STATIC
  output_stream.cpp
)

add_library(gql_ast_print_component INTERFACE)

target_sources(gql_ast_print_component
  INTERFACE
  output_stream.cpp
)

target_link_libraries(gql_ast_print
  gql_ast
)

add_library(gql_ast_test_utils STATIC
  input_positions_test.cpp
)

target_link_libraries(gql_ast_test_utils
  gql_ast
  GTest::gtest_main
)

target_include_directories(gql_ast_test_utils
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
