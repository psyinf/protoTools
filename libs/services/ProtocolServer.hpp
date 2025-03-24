#pragma once
#include <functional>
#include <services/ProtocolMessages.hpp>
#include <services/ProtocolPublisher.hpp>
#include <stop_token>

/**
 * @brief The ProtocolServer class serves a single protocol that is published.
 * It has one publisher and can receive commands via req.
 */

class ProtocolServer
{
public:
    using CommandHandler = std::function<CommandReply(const Command&)>;

    /**
     * @brief Constructs a ProtocolServer with the given protocol header.
     * @param proto The protocol header.
     * @param context The shared pointer to the ZeroMQ context.
     */
    ProtocolServer(const ProtoHeader& proto, std::shared_ptr<zmq::context_t> context);

    /**
     * @brief Binds the server to a publication endpoint.
     * @param pub_endpoint The publication endpoint.
     * @param is_proxy Indicates if the server is a proxy and therefore connects instead of binds.
     */
    void bind(std::string pub_endpoint, bool is_proxy);

    /**
     * @brief Sets the callback function for handling commands.
     * @param callback The command handler callback function.
     */
    void setCommandCallback(CommandHandler callback);

    /**
     * @brief Starts the command handler on the specified endpoint.
     * @param cmd_endpoint The command endpoint.
     */
    void startCommandHandler(const std::string& cmd_endpoint);

    /**
     * @brief Publishes data using the protocol publisher.
     * @param data The data to be published.
     */
    void publish(const ProtoData& data);

private:
    std::shared_ptr<zmq::context_t> _context_ptr;          ///< The ZeroMQ context pointer.
    ProtocolPublisher                  _publisher;            ///< The protocol publisher.
    CommandHandler                  _command_handler = {}; ///< The command handler callback function.
    std::stop_source                _stop_source;          ///< The stop source for stopping the command handler.
    const ProtoHeader               _proto_header;         ///< The protocol header, fixed per server.
};
