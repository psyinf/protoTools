#pragma once
#include <services/ForwardDeclarations.hpp>
#include <services/ProtocolMessages.hpp>
#include <memory>
#include <string>

/**
 * @brief Configuration for the ProtoPublisher.
 */
struct ProtoPublisherConfig
{
    std::string pub_endpoint; ///< Endpoint to bind the publisher to.
    bool        is_proxy{};   ///< If true, the publisher will connect instead of bind.
};

/**
 * @brief A publisher that sends protocols via ZMQ. Should only serve one protocol and one adapter. E.g CAN over CAN-USB, Device 0
 */
class ProtocolPublisher
{
public:
    /**
     * @brief Constructs a ProtoPublisher with the given ZeroMQ context.
     * @param context_ptr The shared pointer to the ZeroMQ context.
     */
    ProtocolPublisher(std::shared_ptr<zmq::context_t> context_ptr);

    /**
     * @brief Destructor for the ProtoPublisher.
     */
    ~ProtocolPublisher();

    /**
     * @brief Binds the publisher to the specified configuration.
     * @param config The configuration for the publisher.
     */
    void bind(const ProtoPublisherConfig& config);

    /**
     * @brief Sends a protocol message consisting of a header and a data package.
     * @param header The protocol header.
     * @param package The protocol data package.
     * @see docs/ProtoHeader.md and ProtoHeader
     */
    void send(const ProtoHeader& header, const ProtoData& package);

private:
    std::unique_ptr<zmq::socket_t>  _pub_socket;  ///< The ZeroMQ publisher socket.
    std::shared_ptr<zmq::context_t> _context_ptr; ///< The shared pointer to the ZeroMQ context.
};