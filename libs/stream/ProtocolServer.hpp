#pragma once
#include <functional>
#include <stream/ProtocolMessages.hpp>
#include <stream/ProtoPublisher.hpp>
#include <stop_token>

// one protocol server should serve one protocol that is published
// it has one publisher and can receive commands via pull
class ProtocolServer
{
public:
    using CommandHandler = std::function<CommandReply(const Command&)>;

    ProtocolServer(const ProtoHeader& proto);

    void bind(std::string pub_endpoint, bool is_proxy);

    void setCommandCallback(CommandHandler callback);

    void startCommandHandler(const std::string& cmd_endpoint);

    void publish(const ProtoData& data);

private:
    std::shared_ptr<zmq::context_t> _context_ptr;
    ProtoPublisher                  _publisher;

    CommandHandler    _command_handler = {};
    std::stop_source  _stop_source;
    const ProtoHeader _proto_header; // fixed per server
};