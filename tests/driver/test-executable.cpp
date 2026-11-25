#include <format>
#include <iostream>
#include <string>
#include <thread>

#include "driver/router.h"

int main(int argc, char* argv[])
{
    if (argc < 1)
        return EXIT_FAILURE;

    std::string name{argv[0]};


    for (int i = 1; i < 6; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::string message = std::format("[{}] Hello World from {}", i, name);
        ::send(STDOUT_FILENO, message.c_str(), message.size(), 0);
    }
    ::send(STDOUT_FILENO, "Goodbye\n", 8, 0);
}
