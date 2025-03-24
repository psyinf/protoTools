#pragma once

#include <services/ProtocolServer.hpp>
#include <services/ProtocolMessages.hpp>
#include <services/directory/ProtocolDirectoryServer.hpp>
#include <services/ProtoUtils.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <format>
#include <spdlog/spdlog.h>

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

private:
    const std::string _protocolName;
    // internal state
    uint8_t _messageCounter{};
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    spdlog::info("Starting Protocol demo server");
    // not needed, but demonstration of ProtocolDirectory
    auto context = ProtoUtils::makeContext(1);
    auto directory = std::make_shared<ProtocolDirectoryServer>(context);
    directory->bind("tcp://*:9999");
    directory->addProtocol({"CAN", "USB_CAN", "tcp://127.0.0.1:41000", "tcp://*:41001"});
    directory->startRunning();

    DemoProtocol p1("CAN");

    ProtocolServer server( context);
    server.bind("tcp://127.0.0.1:41000", false);
    server.setCommandCallback([&p1](const Command& cmd) {
        spdlog::info("Received command: {}:{}:{}", cmd.command_verb, cmd.command_receiver, cmd.command_data);
        return CommandReply{cmd.command_verb, "ACK"};
    });
    server.startCommandHandler("tcp://*:41001");

    spdlog::info("Server started");
    // fire up a proxy
    // zmq::proxy( "inproc://proxy", nullptr);
    while (true)
    {
        auto m = p1.getMessage();
        spdlog::info("Message: {}", m);
        server.publish({"CAN", "CAN_USB"},{std::vector<char>(m.begin(), m.end())});
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