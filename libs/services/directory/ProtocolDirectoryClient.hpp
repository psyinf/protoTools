#pragma once

#include <services/ProtocolMessages.hpp>
#include <services/ForwardDeclarations.hpp>
#include <services/directory/ProtocolDirectoryEntry.hpp>

#include <functional>
#include <memory>
#include <stop_token>

// add change listener
class ProtocolDirectoryClient
{
    using Directory = std::vector<ProtocolDirectoryEntry>;
    using DirectoryUpdateCallback = std::function<void(std::vector<ProtocolDirectoryEntry>)>;

public:
    ProtocolDirectoryClient(std::shared_ptr<zmq::context_t> context_ptr);
    ~ProtocolDirectoryClient();
    void bind(const std::string& endpoint = "tcp://localhost:9999",
              const std::string& pub_endpoint = "tcp://localhost:9998");

    void addChangeCallback(DirectoryUpdateCallback callback) { _directory_update_callback = callback; }

    std::vector<ProtocolDirectoryEntry> queryProtocols();

    // void addProtocol(const ProtocolDirectoryEntry& protocol);
    // void removeProtocol(const std::string& protocol_name);

private:
    void                            updateDirectory(Directory directory);
    std::shared_ptr<zmq::context_t> _context;
    std::shared_ptr<zmq::socket_t>  _req_socket;
    std::shared_ptr<zmq::socket_t>  _sub_socket;
    std::string                     _pub_endpoint;
    std::stop_source                _stop_source;
    Directory                       _directory;
    DirectoryUpdateCallback         _directory_update_callback = {};
};