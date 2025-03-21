#pragma once

#include <stream/ProtocolServer.hpp>
#include <stream/ProtocolMessages.hpp>

#include <iostream>

class DemoProtocol
{
public:
    DemoProtocol(const std::string& protocolName)
      : _protocolName(protocolName)
    {
    }

    std::string getMessage()
    {
        // wait some amount of time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return std::format("Message ({}) {}", _protocolName, _messageCounter++);
    }

    void sendMessage(const std::string& message)
    {
        // send message
        spdlog::info("Sending message: {}", message);
    }

    void run(std::shared_ptr<zmq::context_t> context)
    {
        // start a thread that sends messages
        std::jthread{[&]() {
            zmq::socket_t cmd_socket(*context, zmq::socket_type::push);
            cmd_socket.connect("inproc://proxy");

            while (true)
            {
                sendMessage(getMessage());
            }
        }}.detach();

        
    }

private:
    const std::string _protocolName;
    // internal state
    uint8_t _messageCounter{};
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    DemoProtocol p1("CAN");

    ProtocolServer server({ "CAN" ,"CAN_USB"});
    server.bind("tcp://127.0.0.1:55555", true);
    server.setSendCallback([&p1](const SubscriberCommand& cmd) {
        spdlog::info("Received command: {}:{}:{}", cmd.command_verb, cmd.command_receiver, cmd.command_data);
    });
    server.run("tcp://*:51001");

    spdlog::info("Server started");
    // fire up a proxy
    // zmq::proxy( "inproc://proxy", nullptr);
    while (true)
    {
        auto m = p1.getMessage();
        spdlog::info("Message: {}", m);
        server.send({std::vector<char>(m.begin(), m.end())});
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}

catch (const std::exception& e)
{
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
}

catch (...)
{
    std::cerr << "Unknown exception" << std::endl;
    return 2;
}