//
// Created by andres on 10/28/26.
//

#include "connection.h"

#include <span>

#include "log.h"
#include "protocol.h"
#include "serializer.h"

bool transport::Connection::send(const command::Command& command) const
{
    const command::json j = command;
    const auto serialized = j.dump();
    LOG_TRACE("Sending message {}", serialized);
    const auto span = std::as_bytes(std::span{serialized});
    return protocol::send(this->fd_, span);
}

bool transport::Connection::recv(command::Command& command) const
{
    std::vector<std::byte> bytes;
    if (!protocol::recv(this->fd_, bytes))
        return false;

    LOG_TRACE("Received message: {}", std::string{bytes.begin(), bytes.end()});

    const auto j = command::json::parse(bytes.data(), bytes.data() + bytes.size());
    // const auto j = to_json(bytes);
    command = j;
    return true;
}

void transport::Connection::close()
{
    if (fd_ == -1)
        return;
    ::close(fd_);
    fd_ = -1;
}
