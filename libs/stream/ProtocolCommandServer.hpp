#pragma once

// wraps up a pull socket, that can receive commands and delegate them to a receiver
struct ProtocolCommandServerConfig
{
    std::string endpoint;
};

class ProtocolCommandServer
{
public:
    ProtocolCommandServer(std::shared_ptr<zmq::context_t> context_ptr)
    {
        _context_ptr = context_ptr;
        _pull_cmd_socket = {*_context_ptr, zmq::socket_type::pull};
        _cmd_poller.add(_pull_cmd_socket, zmq::event_flags::pollin);
    }

    void bind(const ProtocolCommandServerConfig& config)
    { //
        _pull_cmd_socket.bind(config.endpoint);
    }

    bool poll(std::chrono::milliseconds timeout)
    {
        std::vector<zmq::poller_event<>> events{1};
        return _cmd_poller.wait_all(events, timeout) > 0;
    }

    SubscriberCommand receive()
    {
        SubscriberCommand cmd;
        zmq::message_t    request_msg;
        std::ignore = _pull_cmd_socket.recv(request_msg);
        cmd.command_verb = std::string(static_cast<char*>(request_msg.data()), request_msg.size());
        std::ignore = _pull_cmd_socket.recv(request_msg);
        cmd.command_receiver = std::string(static_cast<char*>(request_msg.data()), request_msg.size());
        std::ignore = _pull_cmd_socket.recv(request_msg);
        cmd.command_data = std::string(static_cast<char*>(request_msg.data()), request_msg.size());

        spdlog::trace("Received request: {}:{}:{}", cmd.command_verb, cmd.command_receiver, cmd.command_data);
        return cmd;
    }

private:
    zmq::socket_t                   _pull_cmd_socket;
    zmq::poller_t<>                 _cmd_poller;
    std::shared_ptr<zmq::context_t> _context_ptr;
};