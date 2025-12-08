cmake_minimum_required(VERSION 3.22)
project(gql)

enable_testing()

add_subdirectory(src)
add_subdirectory(third_party/fmt)
add_subdirectory(third_party/json)
add_subdirectory(third_party/json-schema-validator)
