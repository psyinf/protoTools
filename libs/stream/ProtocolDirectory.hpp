#pragma once
#include <stream/ForwardDeclarations.hpp>
#include <stream/ProtocolDirectoryEntry.hpp>
#include <memory>
#include <stop_token>
#include <string>
#include <unordered_map>

/* a directory server for for the available protocols. Using a REP socket to answer requests for the available
   protocols. For now we use JSON to describe the available protocols.
*/


class ProtocolDirectory
{
public:
    ProtocolDirectory(std::shared_ptr<zmq::context_t> context_ptr);
    ~ProtocolDirectory();

    void bind(const std::string& endpoint = "tcp://localhost:9999");

    void startRunning();

    void stopRunning();

    void addProtocol(const ProtocolDirectoryEntry& protocol);

private:
    void                                run(std::stop_token st);
    std::shared_ptr<zmq::context_t>     _context_ptr; ///< The ZeroMQ context.
    std::string                         _endpoint;    ///< The endpoint to bind the REP socket to.
    std::stop_source                    _stop_source; ///< The stop source for the running thread.
    std::unordered_map<std::string, ProtocolDirectoryEntry> _protocols;   ///< The available protocols.
    std::string                         _complete_list_json;
};