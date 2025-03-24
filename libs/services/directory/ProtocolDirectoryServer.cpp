#include "ProtocolDirectoryServer.hpp"
#include <zmq.hpp>
#include <nlohmann/json.hpp>

#include <ranges>
#include <algorithm>
#include <thread>

#include <spdlog/spdlog.h>

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProtocolDirectoryEntry,
                                   protocol_name,
                                   adapter_descriptor,
                                   publisher_endpoint,
                                   command_endpoint)

ProtocolDirectoryServer::ProtocolDirectoryServer(std::shared_ptr<zmq::context_t> context_ptr)
  : _context_ptr(context_ptr)
{
    updateCompleteList();
}

ProtocolDirectoryServer::~ProtocolDirectoryServer()
{
    stopRunning();
}

void ProtocolDirectoryServer::bind(const std::string& endpoint /*= "tcp://localhost:9999"*/,
                                   const std::string& publisher_endpoint /* = tcp://localhost:9998*/)
{
    _endpoint = endpoint;
    _publisher_endpoint = publisher_endpoint;
}

void ProtocolDirectoryServer::runRequestServer(std::stop_token st)
{
    auto rep_socket = (std::make_unique<zmq::socket_t>(*_context_ptr, zmq::socket_type::rep));
    // timeout to keep the publisher alive
    rep_socket->set(zmq::sockopt::rcvtimeo, 1000);
    rep_socket->bind(_endpoint);

    auto pub_socket = (std::make_unique<zmq::socket_t>(*_context_ptr, zmq::socket_type::pub));
    pub_socket->bind(_publisher_endpoint);

    while (!st.stop_requested())
    {
        // publish the list of protocols

        pub_socket->send(zmq::message_t(_complete_list_json), zmq::send_flags::none);

        zmq::message_t request;
        auto           size = rep_socket->recv(request);
        if (!size.has_value())
        {
            pub_socket->send(zmq::message_t(_complete_list_json), zmq::send_flags::none);
            continue;
        }

        auto req = request.to_string();
        if (req == LIST_COMMAND)
        { //
            rep_socket->send(zmq::message_t(_complete_list_json), zmq::send_flags::none);
        }
        else if (req == std::string("add"))
        {
            zmq::message_t reply;
            std::ignore = rep_socket->recv(reply);
            auto entry = nlohmann::json::parse(reply.to_string());
            spdlog::info("Adding protocol {}|{}",
                         entry["protocol_name"].get<std::string>(),
                         entry["adapter_descriptor"].get<std::string>());

            addProtocol(entry);
            rep_socket->send(zmq::message_t("OK"), zmq::send_flags::none);
        }
        else if (req == std::string("remove"))
        {
            zmq::message_t reply;
            std::ignore = rep_socket->recv(reply);
            auto entry = nlohmann::json::parse(reply.to_string());
            spdlog::info("Removing protocol {}|{}",
                         entry["protocol_name"].get<std::string>(),
                         entry["adapter_descriptor"].get<std::string>());
            _protocols.erase(entry["protocol_name"].get<std::string>() + "|" +
                             entry["adapter_descriptor"].get<std::string>());
            nlohmann::json j;
            j["protocols"] = nlohmann::json::array();
            std::ranges::for_each(std::views::values(_protocols), [&](auto p) { j["protocols"].push_back(p); });
            _complete_list_json = j.dump();
            rep_socket->send(zmq::message_t("OK"), zmq::send_flags::none);
        }
        else
        {
            rep_socket->send(zmq::message_t(std::format("Unknown request: {}. Valid: {}", req, "list")),
                             zmq::send_flags::none);
        }
    }
}

void ProtocolDirectoryServer::startRunning()
{
    std::jthread j([this]() { runRequestServer(_stop_source.get_token()); });
    j.detach();
}

void ProtocolDirectoryServer::stopRunning()
{
    _stop_source.request_stop();
}

void ProtocolDirectoryServer::addProtocol(const ProtocolDirectoryEntry& protocol)
{
    _protocols[protocol.protocol_name + "|" + protocol.adapter_descriptor] = protocol;
    updateCompleteList();
}

void ProtocolDirectoryServer::updateCompleteList()
{
    nlohmann::json j;
    j["protocols"] = nlohmann::json::array();

    std::ranges::for_each(std::views::values(_protocols), [&](auto p) { j["protocols"].push_back(p); });

    _complete_list_json = j.dump();
}
