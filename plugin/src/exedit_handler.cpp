#include "exedit_handler.h"

namespace mcp {

void ExEditHandler::Init(AviUtl::FilterPlugin* fp) {
    AviUtl::SysInfo si;
    fp->exfunc->get_sys_info(nullptr, &si);
    for (int i = 0; i < si.filter_n; i++) {
        auto filterp = fp->exfunc->get_filterp(i);
        if (strcmp(filterp->name, "拡張編集") == 0 && filterp->information &&
            strcmp(filterp->information,
                   "拡張編集(exedit) version 0.92 by ＫＥＮくん") == 0) {
            exedit_ = filterp;
            break;
        }
    }
    if (!exedit_) {
        return;
    }

    AssignAddr(function_.LoadExo, 0x0004DCA0);
}

}  // namespace mcp
