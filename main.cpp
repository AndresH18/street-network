#include <iostream>
#include <source_location>


// #include "nlohmann/json.hpp"
// #include "protocol/log.h"
// #include "protocol/serializer.h"
#include "protocol/log.h"

// using namespace transport;

namespace hello::super::world {
    class T {
      public:
        void M()
        {

            LOG_DEBUG("Hello World. Testing trace logger from class");
        }
    };
} // namespace hello::super::world

int main()
{
    hello::super::world::T t{};
    t.M();
    // message::Message m{.version = 123, .requestId = 456, .payload = message::AddStreet{"Andres"}};
    // nlohmann::json mj = m;
    // // nlohmann::json j = message::MessageType::Ack;
    // // std::cout << "Hello, World! " << j << std::endl;
    // std::cout << "Message\n" << mj.dump() << std::endl;
    // m = mj;
    // message::json mjj = m;
    // std::cout << "Message\n" << mjj.dump() << std::endl;
    //
    // logger::log_message(logger::LogLevel::Error, mj.dump());
    //
    // logger::error("Worker::run: Invalid FD, hangup, or error: {:#06x}", 123);
    // mj = message::shutdown_message();
    // std::cout << "Shutdown Message: " << mj.dump() << std::endl;
    // m = mj;
    return 0;
}
