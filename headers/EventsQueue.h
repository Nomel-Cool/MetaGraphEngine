#pragma once
#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

/// <summary>
/// 设计：
/// 一对多的线程安全队列
/// </summary>
template<typename T>
class TaskModelQueue 
{
public:
    TaskModelQueue() = default;
    ~TaskModelQueue() = default;
    TaskModelQueue(const TaskModelQueue& other) = delete;
    TaskModelQueue& operator=(const TaskModelQueue&) = delete;

    /// <summary>
    /// 加锁判空，避免判空过程中有取出与放入操作造成影响
    /// </summary>
    /// <returns>空为true，否则false</returns>
    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
        return inner_queue.empty();
    }

    /// <summary>
    /// 当有一个消息入队以后，尝试唤醒任意一个阻塞的获取操作，核心是多个消费者和生产者由Queue的size=0这个点分隔
    /// </summary>
    /// <param name="enlisted_node">事件常量指针</param>
    void AddQuestToQueue(std::unique_ptr<T> enlisted_node)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        inner_queue.push(std::move(enlisted_node));
        cond_.notify_one();
    }

    /// <summary>
    /// 从非空队列中获取一个事件。
    /// 若当前队列为空，条件返回假使加锁操作生效；
    /// 若当前队列不为空，条件返回真，由入队操作唤醒的解锁操作生效。
    /// 避免取走事件后发生异常崩溃，用std::move保证队列内的元素控制权完全交出，避免其它地方错误使用。
    /// </summary>
    /// <returns>返回头部事件</returns>
    std::unique_ptr<T> GetQuestFromQueue()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return !inner_queue.empty(); });
        auto consumed_node = std::move(inner_queue.front());
        inner_queue.pop();
        return consumed_node;
    }

private:
    std::queue<std::unique_ptr<T>> inner_queue;
    std::mutex mutex_;
    std::condition_variable cond_;
};

#endif // !EVENT_QUEUE_H
