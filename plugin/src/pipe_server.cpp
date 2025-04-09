#include "pipe_server.h"

#include <Windows.h>

#include <chrono>
#include <nlohmann/json.hpp>

#include "log.h"
#include "pipe_command.h"

namespace mcp {

using json = nlohmann::json;
using namespace std::literals::chrono_literals;

// Win32APIのHANDLEを閉じるためのカスタムデリータ
struct HandleCloser {
    void operator()(HANDLE handle) const {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
    }
};
// HANDLEをunique_ptrで管理するための型エイリアス
using UniqueHandle =
    std::unique_ptr<std::remove_pointer_t<HANDLE>, HandleCloser>;

// 名前付きパイプサーバークラス
class NamedPipeServer {
  public:
    NamedPipeServer(const std::string& pipe_name);
    ~NamedPipeServer();

    NamedPipeServer(const NamedPipeServer&) = delete;
    NamedPipeServer& operator=(const NamedPipeServer&) = delete;

    NamedPipeServer(NamedPipeServer&&) = delete;
    NamedPipeServer& operator=(NamedPipeServer&&) = delete;

    void Connect(std::stop_token stop_token);
    void Disconnect();

    void Send(const std::string& message);
    std::string Receive(std::stop_token stop_token);

  private:
    static constexpr size_t kBufferSize = 4096;

    HANDLE pipe_ = INVALID_HANDLE_VALUE;
    std::string name_;
    bool connected_ = false;

    bool WaitIo(HANDLE event, std::stop_token stop_token) {
        while (true) {
            DWORD result = WaitForSingleObject(event, 100);
            if (result == WAIT_OBJECT_0) return true;
            if (stop_token.stop_requested()) return false;
        }
    }

    DWORD SendRaw(LPVOID data, DWORD size) {
        UniqueHandle write_event(CreateEventA(nullptr, TRUE, FALSE, nullptr));
        if (!write_event) {
            throw std::runtime_error("CreateEvent failed");
        }
        OVERLAPPED write_ov = {
            .hEvent = write_event.get(),
        };
        WriteFile(pipe_, data, size, nullptr, &write_ov);
        if (!WaitIo(write_event.get(), std::stop_token{})) {
            CancelIoEx(pipe_, &write_ov);
            throw std::runtime_error("WriteFile canceled");
        }
        DWORD written = 0;
        GetOverlappedResult(pipe_, &write_ov, &written, FALSE);
        return written;
    }

    DWORD ReceiveRaw(LPVOID data, DWORD size, std::stop_token stop_token) {
        UniqueHandle read_event(CreateEventA(nullptr, TRUE, FALSE, nullptr));
        if (!read_event) {
            throw std::runtime_error("CreateEvent failed");
        }
        OVERLAPPED read_ov = {
            .hEvent = read_event.get(),
        };
        ReadFile(pipe_, data, size, nullptr, &read_ov);
        if (!WaitIo(read_event.get(), stop_token)) {
            CancelIoEx(pipe_, &read_ov);
            throw std::runtime_error("ReadFile canceled");
        }
        DWORD read = 0;
        GetOverlappedResult(pipe_, &read_ov, &read, FALSE);
        return read;
    }
};

NamedPipeServer::NamedPipeServer(const std::string& pipe_name)
    : name_(pipe_name) {
    pipe_ = CreateNamedPipeA(
        pipe_name.c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1, kBufferSize,
        kBufferSize, 0, nullptr);
    if (pipe_ == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("CreateNamedPipe failed");
    }
}

NamedPipeServer::~NamedPipeServer() {
    if (connected_) {
        Disconnect();
    }
    if (pipe_ != INVALID_HANDLE_VALUE) {
        CloseHandle(pipe_);
        pipe_ = INVALID_HANDLE_VALUE;
    }
}

void NamedPipeServer::Connect(std::stop_token stop_token) {
    UniqueHandle connect_event(CreateEventA(nullptr, TRUE, FALSE, nullptr));
    if (!connect_event) {
        throw std::runtime_error("CreateEvent failed");
    }
    OVERLAPPED connect_ov = {
        .hEvent = connect_event.get(),
    };
    BOOL connected = ConnectNamedPipe(pipe_, &connect_ov);
    if (!connected) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            if (!WaitIo(connect_event.get(), stop_token)) {
                CancelIoEx(pipe_, &connect_ov);
                throw std::runtime_error("Connection canceled");
            }
        } else if (err != ERROR_PIPE_CONNECTED) {
            throw std::runtime_error("ConnectNamedPipe failed");
        }
    }
    connected_ = true;
}

void NamedPipeServer::Disconnect() {
    if (pipe_ != INVALID_HANDLE_VALUE) {
        FlushFileBuffers(pipe_);
        DisconnectNamedPipe(pipe_);
    }
    connected_ = false;
}

void NamedPipeServer::Send(const std::string& message) {
    size_t size = message.size();
    SendRaw(&size, sizeof(size));
    size_t offset = 0;
    while (offset < message.size()) {
        size_t chunk_size = std::min<size_t>(kBufferSize, size - offset);
        size_t written = SendRaw((LPVOID)(message.data() + offset), chunk_size);
        offset += written;
    }
}

std::string NamedPipeServer::Receive(std::stop_token stop_token) {
    size_t size = 0;
    ReceiveRaw(&size, sizeof(size), stop_token);
    if (size == 0) return "";

    std::string message(size, '\0');
    size_t offset = 0;
    while (offset < size) {
        size_t chunk_size = std::min<size_t>(kBufferSize, size - offset);
        size_t read = ReceiveRaw((LPVOID)(message.data() + offset), chunk_size,
                                 stop_token);
        offset += read;
    }
    return message;
}

// 名前付きパイプを使用してPythonのMCPサーバーと通信するスレッド
void PipeServerThread(std::stop_token stop_token, HWND hwnd,
                      ThreadSafeQueue<PipeCommand>& command_queue) {
    Log::Info("Start PipeServerThread");

    while (!stop_token.stop_requested()) {
        try {
            Log::Info("--------------------");
            Log::Info("パイプ作成");
            NamedPipeServer pipe(R"(\\.\pipe\aviutl_mcp_pipe)");
            Log::Info("パイプ接続中");
            pipe.Connect(stop_token);
            Log::Info("パイプ接続完了");

            Log::Info("パイプ受信中");
            auto message = pipe.Receive(stop_token);
            Log::Info("パイプ受信完了");
            json request;
            try {
                request = json::parse(message);
            } catch (const json::parse_error& e) {
                Log::Error("JSON parse error: " + std::string(e.what()));
                json error = {{"error", "Invalid JSON"}};
                std::string error_str = error.dump();
                pipe.Send(error_str);
                pipe.Disconnect();
                continue;
            } catch (const std::exception& e) {
                Log::Error("Exception: " + std::string(e.what()));
                json error = {{"error", "Exception"}};
                std::string error_str = error.dump();
                pipe.Send(error_str);
                pipe.Disconnect();
                continue;
            }

            Log::Info("受信データ: " + message);
            auto promise = std::make_shared<std::promise<json>>();
            std::future<json> future = promise->get_future();
            int id = request.value("id", 0);
            command_queue.Push(PipeCommand{
                .id = id,
                .payload = request,
                .response_promise = promise,
            });
            // メインスレッドに通知
            SendMessage(hwnd, WM_COMMAND_NOTIFY, 0, 0);
            if (future.wait_for(10s) == std::future_status::ready) {
                json response = future.get();
                response["id"] = id;
                std::string response_str = response.dump();
                pipe.Send(response_str);
            } else {
                json timeout = {{"error", "timeout"}, {"id", id}};
                std::string response_str = timeout.dump();
                pipe.Send(response_str);
            }

            pipe.Disconnect();
            Log::Info("パイプ終了");
        } catch (const std::exception& e) {
            Log::Error("NamedPipeServer failed: " + std::string(e.what()));
            break;
        }
    }

    Log::Info("Exit PipeServerThread");
}

}  // namespace mcp
