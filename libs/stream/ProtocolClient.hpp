#pragma once
#include <stream/ProtocolMessages.hpp>
#include <stream/ForwardDeclarations.hpp>
#include <memory>

/**
 * A publisher send protocols via ZMQ. Owns a REP socket to receive requests from the subscriber.
 * The Protoheader is sent with every package and can be used to setup the subscribed topic.
 *
 *
 */

struct ProtoClientConfig
{
    std::string sub_endpoint; ///< Endpoint to bind the subscriber to
    std::string req_endpoint; ///< Endpoint to bind the REQ socket to
};

class ProtoClient
{
public:
    ProtoClient(std::shared_ptr<zmq::context_t> context_ptr);
    ~ProtoClient();

    void bind(const ProtoClientConfig& config);

    void subscribe(const std::string& topic);

    CommandReply sendCommand(const Command& cmd);

    ProtoPackage receiveSubscribed();

private:
    std::unique_ptr<zmq::socket_t>  _sub_socket;
    std::unique_ptr<zmq::socket_t>  _cmd_socket;
    std::shared_ptr<zmq::context_t> _context_ptr;
};