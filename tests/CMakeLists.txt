include(GoogleTest)

add_executable(protocol-test
        protocol/test.cpp)

add_executable(test1 test1.cpp)
add_executable(driver-test-executable driver/test-executable.cpp)

target_link_libraries(test1 PUBLIC protocol-lib driver-lib)
target_link_libraries(driver-test-executable PUBLIC driver-lib protocol-lib)

target_link_libraries(protocol-test PRIVATE
        protocol-lib
        GTest::gtest_main
)

#target_include_directories(transport-test PUBLIC $<TARGET_PROPERTY:transportlib,SOURCE_DIR>)

gtest_discover_tests(protocol-test)


