#include <iostream>
#include <format>
#include <stdexcept>
#include <thread>
#include <services/ProtocolClient.hpp>
#include <services/ProtoUtils.hpp>
#include <services/directory/ProtocolDirectoryClient.hpp>
#include <future>
#include <conio.h>
#include <spdlog/spdlog.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    spdlog::info("Starting Protocol client");
    auto           ctx = ProtoUtils::makeContext(1);
    ProtocolClient sub(ctx);

    ProtocolDirectoryClient dir(ctx);
    dir.bind();
    spdlog::info("Querying protocols from directory");
    auto protocols = dir.queryProtocols(20000);
    std::cout << "Found " << protocols.size() << " protocols" << std::endl;
    for (const auto& p : protocols)
    {
        spdlog::info("Protocol: [{}-{}] publisher:{} | cmd-req: {}",
                     p.protocol_name,
                     p.adapter_descriptor,
                     p.publisher_endpoint,
                     p.command_endpoint);
    }

    
    const std::string sub_endpoint = "tcp://localhost:41000";
    const std::string req_endpoint = "tcp://localhost:41001";
    spdlog::info("Binding to publisher {} and command endpoint {}", sub_endpoint, req_endpoint);
    sub.bind({sub_endpoint, req_endpoint});
    // bind to all topics in protocols
    for (const auto& p : protocols)
    {
        sub.subscribe(p.protocol_name);
        spdlog::info("Subscribed to {}", p.protocol_name);
    }

    auto key = std::async(std::launch::async, []() { return getch(); });

    std::jthread{[&]() {
        while (true)
        {
            auto r = sub.receiveSubscribed();
            std::cout << "Received: " << r.header.protocol_name << "[" << r.header.source << "]" << std::endl;
            std::cout << std::string(r.data.data.begin(), r.data.data.end()) << std::endl;
        }
    }}.detach();

    while (true)
    {
        if (key.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready)
        {
            auto key_char = key.get();
            switch (key_char)
            {
            case 'q': {
                std::cout << "Quit requested" << std::endl;
                return 0;
            }
            case 's': {
                std::string data = std::format("Hullo {}", int(0));
                auto        rep = sub.sendCommand({"SEND", "ECHO", data});
                std::cout << "Sent package, got " << rep.reply_data << std::endl;
                break;
            }
            case 'c': {
                std::string data = std::format("Hullo {}", int(0));
                sub.sendCommand({"CONNECT", "ECHO", data});
                std::cout << "Sent package" << std::endl;
                break;
            }
            case 'a': {
                // send connect to all
                for (const auto& p : protocols)
                {
                    auto rep = sub.sendCommand({"CONNECT", p.protocol_name, ""});
                    spdlog::info("Sent connect to {}, got reply: {}", p.protocol_name, rep.reply_data);
                }
                break;
            }
            default:
                break;
            }
            // re-schedule the async task
            key = std::async(std::launch::async, []() { return getch(); });
        }
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
