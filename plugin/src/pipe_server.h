/*
    PythonのMCPサーバーと名前付きパイプで通信するサーバー
*/

#pragma once

#include <thread>

#include "pipe_command.h"
#include "thread_safe_queue.h"

namespace mcp {

constexpr UINT WM_COMMAND_NOTIFY = WM_USER + 140;

void PipeServerThread(std::stop_token stop_token, HWND hwnd,
                      ThreadSafeQueue<PipeCommand>& command_queue);

}  // namespace mcp
