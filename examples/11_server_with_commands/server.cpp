// Example 11 - ProtocolServer with a command callback.
//
// ProtocolServer = ProtocolPublisher + a REQ/REP command handler running in a
// detached thread. setCommandCallback registers a function invoked for each
// incoming Command; its returned CommandReply is sent back to the client.

#include <services/ProtocolServer.hpp>
#include <services/ProtoUtils.hpp>

#include <atomic>
#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    std::atomic<int> counter{0};

    auto ctx = ProtoUtils::makeContext(1);
    ProtocolServer server(ctx);
    server.bind("tcp://*:5561", /*is_proxy=*/false);
    server.setCommandCallback([&counter](const Command& cmd) -> CommandReply {
        std::cout << "cmd: " << cmd.command_verb << " -> " << cmd.command_receiver
                  << " (" << cmd.command_data << ")\n";
        if (cmd.command_verb == "BUMP") { counter++; return {"ACK", std::to_string(counter.load())}; }
        return {"NACK", "unknown verb"};
    });
    server.startCommandHandler("tcp://*:5562");

    // Publish a heartbeat on the DEMO protocol so a client can observe we're alive.
    for (int i = 0; i < 10; ++i) {
        std::string payload = std::format("heartbeat {} counter={}", i, counter.load());
        server.publish({"DEMO", "srv"}, {std::vector<char>(payload.begin(), payload.end())});
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    return 0;
}
