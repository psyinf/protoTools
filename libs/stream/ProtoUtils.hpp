#pragma once
#include <stream/ForwardDeclarations.hpp>
#include <memory>

class ProtoUtils
{
public:
    std::shared_ptr<zmq::context_t> makeContext(uint8_t num_worker_threads);
};