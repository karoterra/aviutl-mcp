#pragma once

#include <Windows.h>

#include <exception>
#include <format>
#include <string>

namespace mcp {

// UTF-8文字列をstringに変換する
inline std::string ToString(const char8_t* s) {
    return std::string(reinterpret_cast<const char*>(s));
}

// stringをwstringに変換する
// cp: 変換前ののコードページ
inline std::wstring ToWString(const std::string& s, UINT cp) {
    int len = MultiByteToWideChar(cp, 0, s.c_str(), s.length(), nullptr, 0);
    if (len < 0) {
        throw std::runtime_error(
            std::format("failed to convert string (cp:{}) to wstring", cp));
    }
    std::wstring result(len, L'\0');
    MultiByteToWideChar(cp, 0, s.c_str(), s.length(), result.data(), len);
    return result;
}

// wstringをstringに変換する
// cp: 変換先のコードページ
inline std::string ToString(const std::wstring& ws, UINT cp) {
    int len = WideCharToMultiByte(cp, 0, ws.c_str(), ws.length(), nullptr, 0,
                                  nullptr, nullptr);
    if (len < 0) {
        throw std::runtime_error(
            std::format("failed to convert wstring to string (cp:{})", cp));
    }
    std::string result(len, '\0');
    WideCharToMultiByte(cp, 0, ws.c_str(), ws.length(), result.data(), len,
                        nullptr, nullptr);
    return result;
}

// stringのエンコーディングを変換する
// src_cp: 変換前のコードページ
// dst_cp: 変換後のコードページ
inline std::string ConvertEncoding(const std::string& s, UINT src_cp,
                                   UINT dst_cp) {
    auto wide = ToWString(s, src_cp);
    return ToString(wide, dst_cp);
}

}  // namespace mcp
