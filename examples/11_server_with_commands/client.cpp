// Example 11 - Client that sends commands.
//
// Connects to the server's publisher and command endpoints, sends three BUMP
// commands, prints each reply.

#include <services/ProtocolClient.hpp>
#include <services/ProtoUtils.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    auto ctx = ProtoUtils::makeContext(1);
    ProtocolClient client(ctx);
    client.bind({.sub_endpoint = "tcp://localhost:5561",
                 .req_endpoint = "tcp://localhost:5562"});
    client.subscribe("DEMO");

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    for (int i = 1; i <= 3; ++i) {
        auto reply = client.sendCommand({"BUMP", "counter", ""});
        std::cout << "reply: " << reply.reply_verb << " counter=" << reply.reply_data << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    return 0;
}
