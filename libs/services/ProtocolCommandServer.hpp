#pragma once
#include "ForwardDeclarations.hpp"
#include <services/ProtocolMessages.hpp>
#include <memory>
#include <string>

// wraps up a pull socket, that can receive commands and delegate them to a receiver
struct ProtocolCommandServerConfig
{
    std::string endpoint;
};

class ProtocolCommandServer
{
public:
    ProtocolCommandServer(std::shared_ptr<zmq::context_t> context_ptr);

    void bind(const ProtocolCommandServerConfig& config);

    Command receive();

    void acknowledge(CommandReply&& reply);

private:
    std::unique_ptr<zmq::socket_t>  _cmd_socket;
    std::shared_ptr<zmq::context_t> _context_ptr;
};