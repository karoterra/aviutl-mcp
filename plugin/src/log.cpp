#include "log.h"

namespace mcp {

ConsoleColor::ConsoleColor(WORD color) {
    // コンソールのハンドルを取得
    console_ = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console_ == INVALID_HANDLE_VALUE) {
        return;
    }
    // 現在の色を取得
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(console_, &csbi)) {
        console_ = INVALID_HANDLE_VALUE;
        return;
    }
    original_color_ = csbi.wAttributes;
    // 色を変更
    SetConsoleTextAttribute(console_, color);
}

ConsoleColor::~ConsoleColor() {
    // コンソールの色を元に戻す
    if (console_ != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(console_, original_color_);
    }
}

std::string Log::GetTimeStamp() {
    auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    std::chrono::zoned_seconds now_zoned(std::chrono::current_zone(), now_sec);
    return std::format("[{:%H:%M:%S}] ", now_zoned);
}

void Log::Debug(const std::string& message) {
    ConsoleColor color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::cout << GetTimeStamp() << "[MCP][DEBUG]" << message << std::endl;
}

void Log::Info(const std::string& message) {
    std::cout << GetTimeStamp() << "[MCP][INFO]" << message << std::endl;
}

void Log::Warning(const std::string& message) {
    ConsoleColor color(FOREGROUND_RED | FOREGROUND_GREEN |
                       FOREGROUND_INTENSITY);
    std::cout << GetTimeStamp() << "[MCP][WARNING]" << message << std::endl;
}

void Log::Error(const std::string& message) {
    ConsoleColor color(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << GetTimeStamp() << "[MCP][ERROR]" << message << std::endl;
}

std::string Log::GetLastErrorMessage(DWORD error) {
    LPSTR buf;
    DWORD length = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buf), 0, nullptr);
    if (length > 0) {
        std::string message(buf, length);
        LocalFree(buf);
        return message;
    } else {
        return "Unknown error";
    }
}

std::string Log::GetLastErrorMessage() {
    DWORD error = GetLastError();
    return Log::GetLastErrorMessage(error);
}

}  // namespace mcp
