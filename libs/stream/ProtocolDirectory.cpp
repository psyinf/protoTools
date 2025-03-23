#include "ProtocolDirectory.hpp"
#include <zmq.hpp>
#include <nlohmann/json.hpp>

#include <ranges>
#include <algorithm>
#include <thread>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProtocolDirectoryEntry,
                                   protocol_name,
                                   adapter_descriptor,
                                   publisher_endpoint,
                                   command_endpoint)

ProtocolDirectory::ProtocolDirectory(std::shared_ptr<zmq::context_t> context_ptr)
  : _context_ptr(context_ptr)
{
}

ProtocolDirectory::~ProtocolDirectory()
{
    stopRunning();
}

void ProtocolDirectory::bind(const std::string& endpoint /*= "tcp://localhost:9999"*/)
{
    _endpoint = endpoint;
}

void ProtocolDirectory::run(std::stop_token st)
{
    auto rep_socket = (std::make_unique<zmq::socket_t>(*_context_ptr, zmq::socket_type::rep));
    rep_socket->bind(_endpoint);
    while (!st.stop_requested())
    {
        zmq::message_t request;
        std::ignore = rep_socket->recv(request);
        auto req = request.to_string();
        if (req == std::string("list"))
        { //
            rep_socket->send(zmq::message_t(_complete_list_json), zmq::send_flags::none);
        }
        else
        {
            rep_socket->send(zmq::message_t(std::format("Unknown request: {}. Valid: {}", req, "list")),
                             zmq::send_flags::none);
        }
        // later: has-command
    }
}

void ProtocolDirectory::startRunning()
{
    std::jthread j([this]() { run(_stop_source.get_token()); });
}

void ProtocolDirectory::stopRunning()
{
    _stop_source.request_stop();
}

void ProtocolDirectory::addProtocol(const ProtocolDirectoryEntry& protocol)
{
    _protocols[protocol.protocol_name + "|" + protocol.adapter_descriptor] = protocol;
    nlohmann::json j;
    j["protocols"] = nlohmann::json::array();

    std::ranges::for_each(std::views::values(_protocols), [&](auto p) { j["protocols"].push_back(p); });

    _complete_list_json = j.dump();
}
