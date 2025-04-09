/*
    スレッド管理用のコンテナ
*/

#pragma once

#include <thread>
#include <vector>

namespace mcp {

class ThreadManager {
  public:
    ThreadManager() = default;
    ~ThreadManager() = default;

    void Register(std::jthread&& t) { threads_.push_back(std::move(t)); }

    void StopAll() {
        for (auto& t : threads_) {
            t.request_stop();
        }
        for (auto& t : threads_) {
            t.join();
        }
    }

  private:
    std::vector<std::jthread> threads_;
};

}  // namespace mcp
