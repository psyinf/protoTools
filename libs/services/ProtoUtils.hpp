#pragma once
#include <services/ForwardDeclarations.hpp>
#include <memory>

class ProtoUtils
{
public:
    static std::shared_ptr<zmq::context_t> makeContext(uint8_t num_worker_threads);
};