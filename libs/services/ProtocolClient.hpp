#pragma once
#include <services/ProtocolMessages.hpp>
#include <services/ForwardDeclarations.hpp>
#include <memory>

/**
 * @brief Configuration for the ProtocolClient.
 */
struct ProtoClientConfig
{
    std::string sub_endpoint; ///< Endpoint to bind the subscriber to.
    std::string req_endpoint; ///< Endpoint to bind the REQ socket to.
};

/**
 * @brief A client that subscribes to protocols via ZMQ and owns a REQ socket to send requests or commands. (E.g.
 * connect, send)
 * The ProtoHeader is sent with every package and can be used to setup the subscribed topic.
 */
class ProtocolClient
{
public:
    /**
     * @brief Constructs a ProtocolClient with the given ZeroMQ context.
     * @param context_ptr The shared pointer to the ZeroMQ context.
     */
    ProtocolClient(std::shared_ptr<zmq::context_t> context_ptr);

    /**
     * @brief Destructor for the ProtocolClient.
     */
    ~ProtocolClient();

    /**
     * @brief Binds the client to the specified configuration.
     * @param config The configuration for the client.
     */
    void bind(const ProtoClientConfig& config);

    /**
     * @brief Subscribes to a specific topic.
     * @param topic The topic to subscribe to.
     */
    void subscribe(const std::string& topic);

    /**
     * @brief Sends a command and waits for a reply.
     * @param cmd The command to send.
     * @return The reply to the command.
     */
    CommandReply sendCommand(const Command& cmd);

    /**
     * @brief Receives a subscribed protocol package.
     * @return The received protocol package.
     */
    ProtoPackage receiveSubscribed();

private:
    std::unique_ptr<zmq::socket_t>  _sub_socket;  ///< The ZeroMQ subscriber socket.
    std::unique_ptr<zmq::socket_t>  _cmd_socket;  ///< The ZeroMQ command socket.
    std::shared_ptr<zmq::context_t> _context_ptr; ///< The shared pointer to the ZeroMQ context.
};
