#include "ProtocolDirectoryClient.hpp"
#include <services/directory/ProtocolDirectoryEntry.hpp>
#include <nlohmann/json.hpp>
#include <zmq.hpp>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProtocolDirectoryEntry,
                                   protocol_name,
                                   adapter_descriptor,
                                   publisher_endpoint,
                                   command_endpoint)

ProtocolDirectoryClient::ProtocolDirectoryClient(std::shared_ptr<zmq::context_t> context_ptr)
  : _context(context_ptr)
{
}

ProtocolDirectoryClient::~ProtocolDirectoryClient() {}

void ProtocolDirectoryClient::bind(const std::string& endpoint /*= "tcp://localhost:9999"*/,
                                   const std::string& pub_endpoint /*= "tcp://localhost:9998"*/)
{
    _req_socket = std::make_shared<zmq::socket_t>(*_context, zmq::socket_type::req);
    _req_socket->connect(endpoint);
}

std::vector<ProtocolDirectoryEntry> ProtocolDirectoryClient::queryProtocols(uint16_t timeout_ms)
{
    zmq::message_t request(std::string{"list"});
    _req_socket->send(request, zmq::send_flags::none);
    zmq::message_t reply;
    _req_socket->set(zmq::sockopt::rcvtimeo, static_cast<int>(timeout_ms));
    auto opt_val = _req_socket->recv(reply, zmq::recv_flags::none);
    if (!opt_val.has_value()) { return {}; }

    nlohmann::json j = nlohmann::json::parse(reply.to_string());

    Directory received_directory = j["protocols"];
    
    updateDirectory(received_directory);
    return received_directory;
}

void ProtocolDirectoryClient::updateDirectory(Directory directory)
{
    if (directory == _directory) { return; }
    _directory = directory;
    if (_directory_update_callback) { _directory_update_callback(_directory); }
}
