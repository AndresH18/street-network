#include <iostream>
#include <ostream>
#include <unistd.h>

#include "driver/process.h"
#include "protocol/protocol.h"

int main(int argc, char* argv[])
{
    if (argc != 1) {
        for (int i = 0; i < argc; i++) {
            std::cout << argv[i] << std::endl;
        }
        std::exit(EXIT_SUCCESS);
    }

    // std::cout << "start" << std::endl;
    //
    // driver::ProcessConfig config{.name = "test-other", .executable = "./test1", .args{"test-arg"}};
    // driver::Process process = driver::make_process(config);
    // char buffer[transport::protocol::MAX_MESSAGE_SIZE];
    // recv(process.connection().fd(), buffer, transport::protocol::MAX_MESSAGE_SIZE, 0);
    //
    // std::string msg{buffer};
    // std::cout << msg << std::endl;
}
