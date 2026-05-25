// Example 10 - Subscriber.
//
// Connects a ProtocolClient's SUB socket to tcp://localhost:5557, subscribes to
// the "DEMO" topic prefix, and prints 5 received messages. The req_endpoint is
// set to a placeholder - we never call sendCommand in this example.

#include <services/ProtocolClient.hpp>
#include <services/ProtoUtils.hpp>

#include <iostream>
#include <string>

int main()
{
    auto ctx = ProtoUtils::makeContext(1);
    ProtocolClient sub(ctx);
    sub.bind({.sub_endpoint = "tcp://localhost:5557",
              .req_endpoint = "tcp://localhost:5558"}); // placeholder, unused here
    sub.subscribe("DEMO");

    for (int i = 0; i < 5; ++i) {
        auto pkg = sub.receiveSubscribed();
        std::string payload(pkg.data.data.begin(), pkg.data.data.end());
        std::cout << "[" << pkg.header.protocol_name << "|" << pkg.header.source << "] "
                  << payload << "\n";
    }
    return 0;
}
