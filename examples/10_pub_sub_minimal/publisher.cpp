// Example 10 - Publisher.
//
// Binds a ProtocolPublisher on tcp://*:5557 and emits 5 messages under the
// "DEMO" protocol. Sleeps briefly before the first send so a subscriber that
// starts in parallel has time to connect (ZMQ PUB/SUB has a slow-joiner problem
// on connect).

#include <services/ProtocolPublisher.hpp>
#include <services/ProtoUtils.hpp>

#include <chrono>
#include <format>
#include <iostream>
#include <thread>
#include <vector>

int main()
{
    auto ctx = ProtoUtils::makeContext(1);
    ProtocolPublisher pub(ctx);
    pub.bind({.pub_endpoint = "tcp://*:5557"});

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    for (int i = 1; i <= 5; ++i) {
        std::string payload = std::format("tick {}", i);
        pub.send({"DEMO", "loopback"},
                 {std::vector<char>(payload.begin(), payload.end())});
        std::cout << "sent: " << payload << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    return 0;
}
