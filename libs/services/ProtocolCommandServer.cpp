#include "ProtocolCommandServer.hpp"

#include <spdlog/spdlog.h>
#include <zmq.hpp>

 ProtocolCommandServer::ProtocolCommandServer(std::shared_ptr<zmq::context_t> context_ptr)
{
    _context_ptr = context_ptr;
    _cmd_socket = std::make_unique<zmq::socket_t>(*_context_ptr, zmq::socket_type::rep);
}

void ProtocolCommandServer::bind(const ProtocolCommandServerConfig& config)
{
    //
    _cmd_socket->bind(config.endpoint);
}

Command ProtocolCommandServer::receive()
{
    Command        cmd;
    zmq::message_t request_msg;
    std::ignore = _cmd_socket->recv(request_msg);
    cmd.command_verb = std::string(static_cast<char*>(request_msg.data()), request_msg.size());
    std::ignore = _cmd_socket->recv(request_msg);
    cmd.command_receiver = std::string(static_cast<char*>(request_msg.data()), request_msg.size());
    std::ignore = _cmd_socket->recv(request_msg);
    cmd.command_data = std::string(static_cast<char*>(request_msg.data()), request_msg.size());

    spdlog::trace("Received request: {}:{}:{}", cmd.command_verb, cmd.command_receiver, cmd.command_data);
    return cmd;
}

void ProtocolCommandServer::acknowledge(CommandReply&& reply)
{
    _cmd_socket->send(zmq::message_t(reply.reply_verb), zmq::send_flags::sndmore);
    _cmd_socket->send(zmq::message_t(reply.reply_data), zmq::send_flags::none);
}
