#pragma once
#include <stream/ForwardDeclarations.hpp>
#include <stream/ProtocolMessages.hpp>
#include <memory>

/**
 * A publisher send protocols via ZMQ. Should only serve one protocol.

 *
 *
 */
struct ProtoPublisherConfig
{
    std::string pub_endpoint; ///< Endpoint to bind the publisher to
    bool        is_proxy{};   ///< If true, the publisher will connect instead of bind
};

class ProtoPublisher
{
public:
    ProtoPublisher(std::shared_ptr<zmq::context_t> context_ptr);
    ~ProtoPublisher();
    void bind(const ProtoPublisherConfig& config);

    void send(const ProtoHeader& header, const ProtoData& package);

private:
    std::unique_ptr<zmq::socket_t>  _pub_socket;
    std::shared_ptr<zmq::context_t> _context_ptr;
};