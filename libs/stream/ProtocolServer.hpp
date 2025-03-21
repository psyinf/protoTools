#pragma once
#include <thread>
#include <functional>
#include <stream/ProtocolMessages.hpp>
#include <stream/ProtoPublisher.hpp>
#include <stream/ProtocolCommandServer.hpp>

// one protocol server should serve one protocol that is published
// it has one publisher and can receive commands via pull
class ProtocolServer
{
public:
    ProtocolServer(const ProtoHeader& proto)

      : _context_ptr(std::make_unique<zmq::context_t>(1))
      , _publisher(_context_ptr)
      , _proto(proto)

    {
    }

    void bind(std::string pub_endpoint, bool is_proxy) { _publisher.bind({pub_endpoint, is_proxy}); }

    void setSendCallback(std::function<void(const SubscriberCommand&)> callback) { _send_callback = callback; }

    void run(const std::string& cmd_endpoint)
    {
        std::jthread{[&, stop_token = _stop_source.get_token(), endpoint = cmd_endpoint]() {
            ProtocolCommandServer _cmd_server(_context_ptr);
            _cmd_server.bind({endpoint});
            while (!stop_token.stop_requested())
            {
                auto cmd = _cmd_server.receive();
                spdlog::info("Received command: {}:{}:{}", cmd.command_verb, cmd.command_receiver, cmd.command_data);
                if (cmd.command_verb == "SEND") { _send_callback(cmd); }
            }
        }}.detach();
    }

    void send(const ProtoData& data) { _publisher.send(_proto, data); }

private:
    std::shared_ptr<zmq::context_t> _context_ptr;
    ProtoPublisher                  _publisher;

    std::function<void(const SubscriberCommand&)> _send_callback;
    std::stop_source                              _stop_source;
    const ProtoHeader                             _proto;
};