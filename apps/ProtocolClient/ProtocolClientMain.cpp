#include <iostream>
#include <format>
#include <stdexcept>
#include <thread>
#include <zmq.hpp>
#include <stream/ProtocolClient.hpp>
#include <future>
#include <conio.h>
static char input()
{
    return getch();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    std::shared_ptr<zmq::context_t> context = std::make_shared<zmq::context_t>(1);
    ProtoClient                     sub(context);
    sub.bind({"tcp://127.0.0.1:55556", "tcp://127.0.0.1:51001"});
    sub.subscribe("CAN");
    auto key = std::async(std::launch::async, input);
    while (true)
    {
        auto r = sub.receive();
        std::cout << "Received: " << r.header.protocol_name << " " << r.header.adapter_descriptor << std::endl;
        std::cout << std::string(r.data.data.begin(), r.data.data.end()) << std::endl;
        if (key.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
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
                sub.send({"SEND", "CAN", data});
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