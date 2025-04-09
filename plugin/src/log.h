/*
    ログ出力用
*/

#pragma once

#include <Windows.h>

#include <chrono>
#include <format>
#include <iostream>
#include <string>

namespace mcp {

// コンソールの色を変更するクラス
// コンストラクタで色を変更し、デストラクタで元に戻す
class ConsoleColor {
  public:
    ConsoleColor(WORD color);
    ~ConsoleColor();

    // コピー禁止
    ConsoleColor(const ConsoleColor&) = delete;
    ConsoleColor& operator=(const ConsoleColor&) = delete;
    // ムーブ禁止
    ConsoleColor(ConsoleColor&&) = delete;
    ConsoleColor& operator=(ConsoleColor&&) = delete;

  private:
    // コンソールハンドル
    HANDLE console_;
    // 変更前の色
    WORD original_color_;
};

class Log {
  public:
    // コンソール出力用にタイムスタンプを取得
    static std::string GetTimeStamp();

    // コンソールに青色で出力
    static void Debug(const std::string& message);
    // コンソールに白色で出力
    static void Info(const std::string& message);
    // コンソールに黄色で出力
    static void Warning(const std::string& message);
    // コンソールに赤色で出力
    static void Error(const std::string& message);

    // 最後のエラーメッセージを取得
    static std::string GetLastErrorMessage();
    // エラーコードを指定してエラーメッセージを取得
    static std::string GetLastErrorMessage(DWORD error);
};

}  // namespace mcp
