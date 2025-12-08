set(CMAKE_CXX_STANDARD 20)

set(SOURCES
  helpers/check_label_expression.cpp
  helpers/check_label_expression.h
  helpers/path_mode_scope.cpp
  helpers/path_mode_scope.h
  helpers/scope_exit.h
  helpers/split_path_union_by_variable.cpp
  helpers/split_path_union_by_variable.h
  helpers/table_to_json.cpp
  helpers/table_to_json.h
  helpers/variable_bindings_scope.cpp
  helpers/variable_bindings_scope.h
  binding_table.cpp
  binding_table.h
  data_modifying.cpp
  demo.cpp
  demo.h
  error.cpp
  error.h
  graph.cpp
  graph.h
  graph_pattern.cpp
  graph_pattern_context.h
  graph_pattern_planner.cpp
  graph_pattern_planner.h
  gql_program.cpp
  main.cpp
  op_sequence.cpp
  op_sequence.h
  operators.cpp
  operators.h
  predicate.cpp
  query.cpp
  search_context.cpp
  search_context.h
  timeout_watchdog.cpp
  timeout_watchdog.h
  value.h
  value_expression.cpp
)

add_executable(gql_demo
  ${SOURCES}
)

add_executable(gql_demo_dynamic
  ${SOURCES}
)

get_target_property(gql_demo_SOURCES gql_demo SOURCES)
source_group(
  TREE ${CMAKE_CURRENT_SOURCE_DIR}
  PREFIX Sources
  FILES ${gql_demo_SOURCES}
)

target_include_directories(gql_demo
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/cpp-httplib
)

target_include_directories(gql_demo_dynamic
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/cpp-httplib
)

target_link_libraries(gql_demo
  gql_ast_print
  gql_parser
  gql_syntax_analyzer
  nlohmann_json::nlohmann_json
  nlohmann_json_schema_validator
)

target_link_libraries(gql_demo_dynamic
  gql_shared
  nlohmann_json::nlohmann_json
  nlohmann_json_schema_validator
)

add_executable(gql_demo_test
  helpers/split_path_union_by_variable.cpp
  helpers/split_path_union_by_variable.h
  helpers/split_path_union_by_variable_test.cpp
)

include(GoogleTest)
gtest_discover_tests(gql_demo_test)

target_link_libraries(gql_demo_test
  gql_parser
  gql_ast_print
  GTest::gtest_main
  GTest::gmock_main
)
