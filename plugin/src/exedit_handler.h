/*
    拡張編集を操作するためのクラス

    参考

    アルティメットプラグイン
    https://github.com/hebiiro/anti.aviutl.ultimate.plugin/
    MIT License
    Copyright (c) 2023 hebiiro
*/

#pragma once

#include <aviutl.hpp>
#include <cstdint>

namespace mcp {

class ExEditHandler {
  public:
    void Init(AviUtl::FilterPlugin* fp);

    bool IsAvailable() const { return exedit_ != nullptr; }

    BOOL LoadExo(LPCSTR file_name, int32_t frame, int32_t layer,
                 AviUtl::EditHandle* editp) {
        if (!IsAvailable()) {
            return FALSE;
        }
        return function_.LoadExo(file_name, frame, layer, exedit_, editp);
    }

  private:
    AviUtl::FilterPlugin* exedit_ = nullptr;

    struct Function {
        BOOL(CDECL* LoadExo)(LPCSTR file_name, int32_t frame, int32_t layer,
                             AviUtl::FilterPlugin* fp,
                             AviUtl::EditHandle* editp) = nullptr;
    } function_;

    template <typename T>
    void AssignAddr(T& p, uintptr_t offset) {
        if (!IsAvailable()) {
            return;
        }
        auto base = reinterpret_cast<uintptr_t>(exedit_->dll_hinst);
        p = reinterpret_cast<T>(base + offset);
    }

    template <typename T>
    T* GetPtr(uintptr_t offset) {
        if (!IsAvailable()) {
            return nullptr;
        }
        auto base = reinterpret_cast<uintptr_t>(exedit_->dll_hinst);
        auto ptr = reinterpret_cast<T*>(base + offset);
        return ptr;
    }
};

}  // namespace mcp
