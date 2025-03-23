#pragma once

#include <stream/ProtocolMessages.hpp>
#include <stream/ForwardDeclarations.hpp>
#include <stream/ProtocolDirectoryEntry.hpp>
#include <memory>

class ProtocolDirectoryClient
{
public:
    ProtocolDirectoryClient(std::shared_ptr<zmq::context_t> context_ptr);
    ~ProtocolDirectoryClient();
    void                      bind(const std::string& endpoint = "tcp://localhost:9999");

    std::vector<ProtocolDirectoryEntry> queryProtocols();
private:
    std::shared_ptr<zmq::context_t> _context;
    std::shared_ptr<zmq::socket_t>  _socket;
};