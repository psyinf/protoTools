#include "ProtoPublisher.hpp"
#include <spdlog/spdlog.h>
#include <zmq.hpp>

namespace {
static zmq::message_t makeHeaderMessage(const ProtoHeader& header)
{
    std::string header_str = header.protocol_name;
    if (!header.adapter_descriptor.empty()) { header_str += "|" + header.adapter_descriptor; }
    return zmq::message_t(header_str);
}

static zmq::message_t makeProtoMessage(const ProtoData& protoPack)
{
    return zmq::message_t(protoPack.data.data(), protoPack.data.size());
}
} // namespace

ProtoPublisher::~ProtoPublisher() = default;

ProtoPublisher::ProtoPublisher(std::shared_ptr<zmq::context_t> context_ptr)
{
    _context_ptr = context_ptr;
    _pub_socket = std::make_unique<zmq::socket_t>(*_context_ptr, zmq::socket_type::pub);
}

void ProtoPublisher::bind(const ProtoPublisherConfig& config)
{
    // when we use a proxy, we need to connect instead of bind
    if (config.is_proxy) { _pub_socket->connect(config.pub_endpoint); }
    else { _pub_socket->bind(config.pub_endpoint); }
}

void ProtoPublisher::send(const ProtoHeader& header, const ProtoData& package)
{
    // format header
    _pub_socket->send(makeHeaderMessage(header), zmq::send_flags::sndmore);
    _pub_socket->send(makeProtoMessage(package), zmq::send_flags::none);
    spdlog::trace("Sent package");
}
