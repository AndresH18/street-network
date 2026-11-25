add_library(driver-lib
        process.cpp
        process.h
        process_manager.cpp
        process_manager.h
        router.cpp
        router.h
        connection_loop.cpp
        connection_loop.h
)

target_include_directories(driver-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(driver-lib PUBLIC protocol-lib)

add_executable(driver-main driver-main.cpp)
target_link_libraries(driver-main PRIVATE driver-lib)

target_compile_definitions(driver-lib PUBLIC
        PY_EXECUTABLE="${CMAKE_SOURCE_DIR}/py/main.py"
        RGEN_EXECUTABLE="$<TARGET_FILE:rgen-main>"
        SP_EXECUTABLE="$<TARGET_FILE:short-path-main>"
)

add_dependencies(driver-main short-path-main rgen-main)
