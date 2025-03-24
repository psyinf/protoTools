#include <iostream>
#include <format>
#include <stdexcept>
#include <thread>
#include <services/ProtocolClient.hpp>
#include <services/ProtoUtils.hpp>
#include <services/directory/ProtocolDirectoryClient.hpp>
#include <future>
#include <conio.h>

static char input()
{
    return getch();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    auto           ctx = ProtoUtils::makeContext(1);
    ProtocolClient                     sub(ctx );
//     ProtocolDirectoryClient dir(ctx);
//     dir.bind();
//     auto protocols = dir.queryProtocols();
//     for (const auto& p : protocols)
//     {
//         std::format("Protocol: {} {} at pub:{}| cmd:{}", p.protocol_name, p.adapter_descriptor, p.publisher_endpoint, p.command_endpoint);
//     }

    //sub.bind({"tcp://127.0.0.1:55556", "tcp://127.0.0.1:51001"});
    sub.bind({ "tcp://127.0.0.1:41000", "tcp://127.0.0.1:41001" });
    sub.subscribe("ECHO");
    auto key = std::async(std::launch::async, input);

    std::jthread{[&]() {
        while (true)
        {
            auto r = sub.receiveSubscribed();
            std::cout << "Received: " << r.header.protocol_name << " " << r.header.adapter_descriptor << std::endl;
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
            case 'q':
            {
                std::cout << "Quit requested" << std::endl;
                return 0;
            }
            case 's': {
                std::string data = std::format("Hullo {}", int(0));
                auto rep = sub.sendCommand({"SEND", "ECHO", data});
                std::cout << "Sent package, got " << rep.reply_data << std::endl;
                break;
            }
            case 'c': {
                std::string data = std::format("Hullo {}", int(0));
                sub.sendCommand({"CONNECT", "ECHO", data});
                std::cout << "Sent package" << std::endl;
                break;
            }
            default:
                break;
            }
            // re-schedule the async task
            key = std::async(std::launch::async, input);
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
