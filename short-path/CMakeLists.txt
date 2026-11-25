add_library(short-path-lib
        graph.cpp
        graph.h
        graph.cpp
        graph.h
)

target_include_directories(short-path-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(short-path-lib PUBLIC protocol-lib)

add_executable(short-path-main sp-main.cpp)

target_link_libraries(short-path-main PRIVATE short-path-lib)
