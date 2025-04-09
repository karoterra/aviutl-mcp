/*
    スレッドセーフなキュー
*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace mcp {

template <typename T>
class ThreadSafeQueue {
  public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() { Stop(); }

    // コピーコンストラクタと代入演算子は削除
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void Push(const T& value) {
        {
            std::scoped_lock lock(mutex_);
            queue_.push(value);
        }
        cv_.notify_one();
    }

    std::optional<T> Pop() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });

        if (stopped_ && queue_.empty()) {
            return std::nullopt;
        }

        T value = queue_.front();
        queue_.pop();
        return value;
    }

    void Stop() {
        {
            std::scoped_lock lock(mutex_);
            stopped_ = true;
        }
        cv_.notify_all();
    }

    bool Empty() const {
        std::scoped_lock lock(mutex_);
        return queue_.empty();
    }

  private:
    mutable std::mutex mutex_;
    std::queue<T> queue_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

}  // namespace mcp
