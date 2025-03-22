#include "ProtoUtils.hpp"
#include <zmq.hpp>

std::shared_ptr<zmq::context_t> ProtoUtils::makeContext(uint8_t num_worker_threads)
{
    return std::make_shared<zmq::context_t>(num_worker_threads);
}