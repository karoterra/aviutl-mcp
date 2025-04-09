/*
    MCPプラグインのAviUtlに公開する部分
*/

#include <aviutl.hpp>

#include "mcp_plugin.h"

// プラグインの名前と説明
const char* kFilterName = "MCP";
const char* kFilterInfo = "MCPプラグイン v0.0.0 by karoterra";

mcp::McpFilterPlugin mcp_filter;

BOOL func_init(AviUtl::FilterPlugin* fp) { return mcp_filter.Init(fp); }
BOOL func_exit(AviUtl::FilterPlugin* fp) { return mcp_filter.Exit(fp); }

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam,
                  AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp) {
    return mcp_filter.WndProc(hwnd, message, wparam, lparam, editp, fp);
}

using AviUtl::detail::FilterPluginFlag;

AviUtl::FilterPluginDLL filter_src{
    .flag = FilterPluginFlag::AlwaysActive | FilterPluginFlag::PriorityLowest |
            FilterPluginFlag::ExInformation | FilterPluginFlag::DispFilter |
            FilterPluginFlag::WindowSize,
    .x = 100,
    .y = 100,
    .name = kFilterName,
    .func_init = func_init,
    .func_exit = func_exit,
    .func_WndProc = func_WndProc,
    .information = kFilterInfo,
};

extern "C" __declspec(dllexport) AviUtl::FilterPluginDLL* GetFilterTable(void) {
    return &filter_src;
}
