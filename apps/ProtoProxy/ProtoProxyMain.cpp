//later 
#include <iostream>
#include <format>
#include <stdexcept>
#include <thread>
#include <zmq.hpp>
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
try
{
    std::shared_ptr<zmq::context_t> context = std::make_shared<zmq::context_t>(1);
    // producer facing frontend socket
    zmq::socket_t frontend(*context, zmq::socket_type::xsub);

    // consumer facing backend socket
    zmq::socket_t backend(*context, zmq::socket_type::xpub);

    // bind frontend to tcp://*:5555
    frontend.bind("tcp://*:55555");
    backend.bind("tcp://*:55556");

    // start proxy
    zmq::proxy(frontend, backend, nullptr);
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