set(SOURCES
    error.cpp
    features.cpp
)

add_library(gql_common STATIC
    ${SOURCES}
)

add_library(gql_common_component INTERFACE)

target_sources(gql_common_component
    INTERFACE
    ${SOURCES}
)

target_include_directories(gql_common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/fmt/include
)

target_include_directories(gql_common_component
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/fmt/include
)

target_link_libraries(gql_common fmt::fmt)
target_link_libraries(gql_common_component INTERFACE fmt::fmt-header-only)
