add_library(protocol-lib STATIC
        protocol.h
        connection.cpp
        connection.h
        serializer.h
        worker.h
        log.h
        command.h
        command_dispatcher.h
)

include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

target_include_directories(protocol-lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(protocol-lib PUBLIC nlohmann_json::nlohmann_json)
