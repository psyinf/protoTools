#include <iostream>
#include <format>
#include <stdexcept>
#include <thread>
#include <stream/ProtoPublisher.hpp>
#include <stream/ProtoUtils.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    ProtoUtils     utils;
    ProtoPublisher pub1(utils.makeContext(1));
    pub1.bind({"tcp://*:5555"});
    
    size_t counter = 0;
    while (true)
    {
        std::string data = std::format("Hullo {}", int(counter++));

        pub1.send({ "CAN", "USB-CAN" }, { {std::vector<char>(data.begin(), data.end())}});
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
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