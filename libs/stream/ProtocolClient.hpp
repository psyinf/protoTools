#pragma once
#include <zmq.h>
#include <spdlog/spdlog.h>
#include <stream/ProtocolMessages.hpp>

/**
 * A publisher send protocols via ZMQ. Owns a REP socket to receive requests from the subscriber.
 * The Protoheader is sent with every package and can be used to setup the subscribed topic.
 *
 *
 */

struct ProtoClientConfig
{
    std::string sub_endpoint; ///< Endpoint to bind the subscriber to
    std::string req_endpoint; ///< Endpoint to bind the REQ socket to
};

class ProtoClient
{
public:
    ProtoClient(std::shared_ptr<zmq::context_t> context_ptr)
    {
        _context_ptr = context_ptr;
        _sub_socket = {*context_ptr, zmq::socket_type::sub};
        _cmd_socket = {*_context_ptr, zmq::socket_type::push};
    }

    void bind(const ProtoClientConfig& config)
    {
        _sub_socket.connect(config.sub_endpoint);
        _cmd_socket.connect(config.req_endpoint);
    }

    void subscribe(const std::string& topic) { _sub_socket.set(zmq::sockopt::subscribe, topic); }

    void send(const SubscriberCommand& cmd)
    {
        // format header
        
        _cmd_socket.send(zmq::message_t(cmd.command_verb), zmq::send_flags::sndmore);
        _cmd_socket.send(zmq::message_t(cmd.command_receiver), zmq::send_flags::sndmore);
        _cmd_socket.send(zmq::message_t(cmd.command_data), zmq::send_flags::none);
        spdlog::trace("Sent SubscriberRequest");
    }

    ProtoPackage receive()
    {
        zmq::message_t header_msg;
        std::ignore = _sub_socket.recv(header_msg);
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

        std::ignore = _sub_socket.recv(package_msg);
        ProtoData data{.data{std::vector<char>(static_cast<char*>(package_msg.data()),
                                               static_cast<char*>(package_msg.data()) + package_msg.size())}};
        spdlog::info("Received package");

        return ProtoPackage{.header{header}, .data{data}};
    }

private:
    zmq::socket_t                   _sub_socket;
    zmq::socket_t                   _cmd_socket;
    std::shared_ptr<zmq::context_t> _context_ptr;
};