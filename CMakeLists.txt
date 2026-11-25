cmake_minimum_required(VERSION 4.2)
project(ece650-a4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # enforce C++ Standard 20
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

option(ENABLE_TESTS "Enable tests" OFF)

add_executable(hw main.cpp)
target_link_libraries(hw PRIVATE protocol-lib)

add_subdirectory(protocol)
add_subdirectory(driver)
add_subdirectory(short-path)
add_subdirectory(r-gen)

install(TARGETS driver-main rgen-main short-path-main DESTINATION bin)
install(PROGRAMS py/main.py py/communication.py py/segmenter.py DESTINATION bin)

if (ENABLE_TESTS)
    enable_testing()
    include(FetchContent)

    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    )

    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif ()

