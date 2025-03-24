#pragma once
#include <services/ForwardDeclarations.hpp>
#include <services/directory/ProtocolDirectoryEntry.hpp>
#include <memory>
#include <stop_token>
#include <string>
#include <unordered_map>

/* a directory server for for the available protocols. Using a REP socket to answer requests for the available
   protocols. For now we use JSON to describe the available protocols.
*/

class ProtocolDirectoryServer
{
    static constexpr auto receive_timeout_ms = 1000;
    static constexpr auto LIST_COMMAND = "list";
    static constexpr auto ADD_COMMAND = "add";
    static constexpr auto REMOVE_COMMAND = "remove";

    using ProtocolDirectory = std::unordered_map<std::string, ProtocolDirectoryEntry>;

public:
    ProtocolDirectoryServer(std::shared_ptr<zmq::context_t> context_ptr);
    ~ProtocolDirectoryServer();

    void bind(const std::string& req_endpoint = "tcp://localhost:9999",
              const std::string& publisher_endpoint = "tcp://localhost:9998");

    void startRunning();

    void stopRunning();
    /**
     * @brief Add a protocol to the directory directly.
     * @param protocol
     */
    void addProtocol(const ProtocolDirectoryEntry& protocol);

private:
    void runRequestServer(std::stop_token st);
    void updateCompleteList();

    std::shared_ptr<zmq::context_t> _context_ptr;        ///< The ZeroMQ context.
    std::string                     _endpoint;           ///< The endpoint to bind the REP socket to.
    std::string                     _publisher_endpoint; ///< The endpoint to bind the PUB socket to.
    std::stop_source                _stop_source;        ///< The stop source for the running thread.
    ProtocolDirectory               _protocols;          ///< The available protocols.
    std::string                     _complete_list_json;
};