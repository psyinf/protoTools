#include "ProtocolServer.hpp"
#include <spdlog/spdlog.h>
#include <zmq.hpp>
#include <ProtocolCommandServer.hpp>

ProtocolServer::ProtocolServer(std::shared_ptr<zmq::context_t> context)
  : _context_ptr(context)
  , _publisher(_context_ptr)
{
}

void ProtocolServer::bind(std::string pub_endpoint, bool is_proxy)
{
    _publisher.bind({pub_endpoint, is_proxy});
}

void ProtocolServer::setCommandCallback(CommandHandler callback)
{
    _command_handler = callback;
}

void ProtocolServer::startCommandHandler(const std::string& cmd_endpoint)
{
    std::jthread{[&, stop_token = _stop_source.get_token(), endpoint = cmd_endpoint]() {
        ProtocolCommandServer _cmd_server(_context_ptr);
        _cmd_server.bind({endpoint});
        while (!stop_token.stop_requested())
        {
            auto cmd = _cmd_server.receive();
            spdlog::trace("Received command: {}:{}:{}", cmd.command_verb, cmd.command_receiver, cmd.command_data);
            _cmd_server.acknowledge(_command_handler(cmd));
        }
    }}.detach();
}

void ProtocolServer::publish(const ProtoHeader& proto_header, const ProtoData& data)
{
    _publisher.send(proto_header, data);
}
