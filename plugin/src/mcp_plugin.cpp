#include "mcp_plugin.h"

#include <string>

#include "log.h"
#include "pipe_server.h"
#include "string_helper.h"

namespace mcp {

BOOL McpFilterPlugin::Init(AviUtl::FilterPlugin* fp) {
    exedit_.Init(fp);
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
    BOOL result = FALSE;
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
                result = GetProjectInfo(fp, editp, res);
            } else if (method == "add_object") {
                auto filename = ConvertEncoding(
                    cmd.payload["params"].value("filename", ""), CP_UTF8, 932);
                auto frame = cmd.payload["params"].value("frame", 0);
                auto layer = cmd.payload["params"].value("layer", 0);
                result = AddObject(editp, filename, frame, layer, res);
            } else {
                res["error"] = "Unknown method";
            }

            cmd.response_promise->set_value(res);

            return result;
        }
    }
    return FALSE;
}

BOOL McpFilterPlugin::GetProjectInfo(AviUtl::FilterPlugin* fp,
                                     AviUtl::EditHandle* editp, json& res) {
    AviUtl::FileInfo fi;
    fp->exfunc->get_file_info(editp, &fi);
    res["width"] = fi.w;
    res["height"] = fi.h;
    res["audioRate"] = fi.audio_rate;
    res["audioCh"] = fi.audio_ch;
    res["frameRate"] = static_cast<double>(fi.video_rate) /
                       static_cast<double>(fi.video_scale);
    return FALSE;
}

BOOL McpFilterPlugin::AddObject(AviUtl::EditHandle* editp,
                                const std::string& filename, int32_t frame,
                                int32_t layer, json& res) {
    exedit_.LoadExo(filename.c_str(), frame, layer, editp);
    res["result"] = ToString(u8"オブジェクトを追加しました");
    return TRUE;
}

}  // namespace mcp
