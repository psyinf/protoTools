#include "ProtocolDirectoryClient.hpp"
#include <stream/ProtocolDirectoryEntry.hpp>
#include <nlohmann/json.hpp>
#include <zmq.hpp>


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProtocolDirectoryEntry,
                                   protocol_name,
                                   adapter_descriptor,
                                   publisher_endpoint,
                                   command_endpoint)



ProtocolDirectoryClient::ProtocolDirectoryClient(std::shared_ptr<zmq::context_t> context_ptr): _context(context_ptr) {}

ProtocolDirectoryClient::~ProtocolDirectoryClient() {}

 void ProtocolDirectoryClient::bind(const std::string& endpoint /*= "tcp://localhost:9999"*/) {
     _socket = std::make_shared<zmq::socket_t>(*_context, zmq::socket_type::req);
     _socket->connect(endpoint);
 }

std::vector<ProtocolDirectoryEntry> ProtocolDirectoryClient::queryProtocols() {

    zmq::message_t request(std::string{ "list" });
    _socket->send(request, zmq::send_flags::none);
    zmq::message_t reply;
    _socket->recv(reply, zmq::recv_flags::none);
   
    nlohmann::json            j = nlohmann::json::parse(reply.to_string());

    std::vector<ProtocolDirectoryEntry> protocols;
    for (const auto& entry : j["protocols"])
    {
        ProtocolDirectoryEntry p = entry;
        protocols.push_back(p);
    }

    return protocols;
}
