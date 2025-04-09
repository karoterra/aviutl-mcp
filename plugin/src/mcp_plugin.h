/*
    MCPプラグイン
*/

#pragma once

#include <aviutl.hpp>
#include <thread>

#include "pipe_command.h"
#include "thread_manager.h"
#include "thread_safe_queue.h"

namespace mcp {

class McpFilterPlugin {
  public:
    McpFilterPlugin() = default;
    ~McpFilterPlugin() = default;

    BOOL Init(AviUtl::FilterPlugin* fp);
    BOOL Exit(AviUtl::FilterPlugin* fp);
    BOOL WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
                 AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);

  private:
    ThreadManager thread_manager_;
    ThreadSafeQueue<PipeCommand> command_queue_;
};

}  // namespace mcp
