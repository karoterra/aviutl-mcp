#pragma once

#include <Windows.h>

#include <future>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace mcp {

using json = nlohmann::json;

struct PipeCommand {
    int id;
    json payload;
    std::shared_ptr<std::promise<json>> response_promise;
};

}  // namespace mcp
