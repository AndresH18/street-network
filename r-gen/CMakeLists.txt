add_library(rgen-lib
        street.h
        street.cpp
        segment.h
        point.h
        geometry.cpp
        geometry.h
        generator.cpp
        generator.h
        random.h
)

target_include_directories(rgen-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(rgen-lib PUBLIC protocol-lib)

add_executable(rgen-main rgen-main.cpp)
target_link_libraries(rgen-main PRIVATE rgen-lib)
