#include "mcp_plugin.h"

#include "log.h"
#include "pipe_server.h"

namespace mcp {

BOOL McpFilterPlugin::Init(AviUtl::FilterPlugin* fp) {
    thread_manager_.Register(
        std::jthread{PipeServerThread, fp->hwnd, std::ref(command_queue_)});
    return TRUE;
}

BOOL McpFilterPlugin::Exit(AviUtl::FilterPlugin* fp) {
    command_queue_.Stop();
    thread_manager_.StopAll();
    return TRUE;
}

BOOL McpFilterPlugin::WndProc(HWND hwnd, UINT message, WPARAM wparam,
                              LPARAM lparam, AviUtl::EditHandle* editp,
                              AviUtl::FilterPlugin* fp) {
    switch (message) {
        case WM_COMMAND_NOTIFY: {
            auto opt = command_queue_.Pop();
            if (!opt) {
                return FALSE;
            }

            const auto& cmd = *opt;
            json res;
            std::string method = cmd.payload.value("method", "");
            if (method == "get_project_info") {
                AviUtl::FileInfo fi;
                fp->exfunc->get_file_info(editp, &fi);
                res["width"] = fi.w;
                res["height"] = fi.h;
                res["audioRate"] = fi.audio_rate;
                res["audioCh"] = fi.audio_ch;
                res["frameRate"] = static_cast<double>(fi.video_rate) /
                                   static_cast<double>(fi.video_scale);
            } else {
                res["error"] = "Unknown method";
            }

            cmd.response_promise->set_value(res);

            return FALSE;
        }
    }
    return FALSE;
}

}  // namespace mcp
