/*
    MCPプラグイン
*/

#pragma once

#include <aviutl.hpp>
#include <thread>

#include "exedit_handler.h"
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

    BOOL GetProjectInfo(AviUtl::FilterPlugin* fp, AviUtl::EditHandle* editp,
                        json& res);
    BOOL AddObject(AviUtl::EditHandle* editp, const std::string& filename,
                   int32_t frame, int32_t layer, json& res);

  private:
    ThreadManager thread_manager_;
    ThreadSafeQueue<PipeCommand> command_queue_;
    ExEditHandler exedit_;
};

}  // namespace mcp
