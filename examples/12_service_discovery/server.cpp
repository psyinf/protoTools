// Example 12 - Directory server.
//
// Registers a couple of known protocols in the directory, binds the REP/PUB
// endpoints, and runs the request-response loop on a detached thread. A client
// (see client.cpp) can then query `list` to retrieve them.

#include <services/ProtoUtils.hpp>
#include <services/directory/ProtocolDirectoryServer.hpp>

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    auto ctx = ProtoUtils::makeContext(1);
    ProtocolDirectoryServer dir(ctx);
    dir.bind("tcp://*:9999", "tcp://*:9998");

    dir.addProtocol({.protocol_name      = "CAN",
                     .adapter_descriptor = "USB_CAN",
                     .publisher_endpoint = "tcp://127.0.0.1:41000",
                     .command_endpoint   = "tcp://127.0.0.1:41001"});
    dir.addProtocol({.protocol_name      = "MODBUS",
                     .adapter_descriptor = "TCP",
                     .publisher_endpoint = "tcp://127.0.0.1:42000",
                     .command_endpoint   = "tcp://127.0.0.1:42001"});

    dir.startRunning();
    std::cout << "directory server running. Ctrl+C to exit.\n";

    // Idle loop; the directory runs on its own thread.
    while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }
    return 0;
}
