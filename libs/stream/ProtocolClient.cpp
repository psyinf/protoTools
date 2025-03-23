#include "ProtocolClient.hpp"
#include <spdlog/spdlog.h>
#include <zmq.hpp>
ProtocolClient::~ProtocolClient() = default;

ProtocolClient::ProtocolClient(std::shared_ptr<zmq::context_t> context_ptr)
{
    _context_ptr = context_ptr;
    _sub_socket = std::make_unique<zmq::socket_t>(*context_ptr, zmq::socket_type::sub);
    _cmd_socket = std::make_unique<zmq::socket_t>(*_context_ptr, zmq::socket_type::req);
}

void ProtocolClient::bind(const ProtoClientConfig& config)
{
    _sub_socket->connect(config.sub_endpoint);
    _cmd_socket->connect(config.req_endpoint);
}

void ProtocolClient::subscribe(const std::string& topic)
{
    _sub_socket->set(zmq::sockopt::subscribe, topic);
}

CommandReply ProtocolClient::sendCommand(const Command& cmd)
{
    // format header
    _cmd_socket->send(zmq::message_t(cmd.command_verb), zmq::send_flags::sndmore);
    _cmd_socket->send(zmq::message_t(cmd.command_receiver), zmq::send_flags::sndmore);
    _cmd_socket->send(zmq::message_t(cmd.command_data), zmq::send_flags::none);
    spdlog::trace("Sent SubscriberRequest");
    
    CommandReply   reply;
    zmq::message_t reply_msg;
    std::ignore = _cmd_socket->recv(reply_msg);
    reply.reply_verb = std::string(static_cast<char*>(reply_msg.data()), reply_msg.size());
    std::ignore = _cmd_socket->recv(reply_msg);
    reply.reply_data = std::string(static_cast<char*>(reply_msg.data()), reply_msg.size());
    return reply;
}

ProtoPackage ProtocolClient::receiveSubscribed()
{
    zmq::message_t header_msg;
    std::ignore = _sub_socket->recv(header_msg);
    ProtoHeader header;
    std::string header_str(static_cast<char*>(header_msg.data()), header_msg.size());
    // split on | if present
    auto pos = header_str.find('|');
    if (pos != std::string::npos)
    {
        header.protocol_name = header_str.substr(0, pos);
        header.adapter_descriptor = header_str.substr(pos + 1);
    }
    else { header.protocol_name = header_str; }

    // get first part of message
    zmq::message_t package_msg;

    std::ignore = _sub_socket->recv(package_msg);
    ProtoData data{.data{std::vector<char>(static_cast<char*>(package_msg.data()),
                                           static_cast<char*>(package_msg.data()) + package_msg.size())}};
    spdlog::trace("Received package: {}", package_msg.to_string());

    return ProtoPackage{.header{header}, .data{data}};
}
