// Example 12 - Directory client.
//
// Queries the server for the registered protocols and prints them.

#include <services/ProtoUtils.hpp>
#include <services/directory/ProtocolDirectoryClient.hpp>

#include <iostream>

int main()
{
    auto ctx = ProtoUtils::makeContext(1);
    ProtocolDirectoryClient dir(ctx);
    dir.bind("tcp://localhost:9999", "tcp://localhost:9998");

    auto protocols = dir.queryProtocols(/*timeout_msec=*/2000);
    std::cout << "found " << protocols.size() << " protocols:\n";
    for (const auto& p : protocols) {
        std::cout << "  " << p.protocol_name << "|" << p.adapter_descriptor
                  << "  pub=" << p.publisher_endpoint
                  << "  cmd=" << p.command_endpoint << "\n";
    }
    return 0;
}
