#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <future>

#include "EventsQueue.h"

class ShabbyThreadPool {
public:
    // 获取单例实例
    static ShabbyThreadPool& GetInstance();

    /// <summary>
    /// 接受F(Args...)的可调用类型，并签发一个F()的future的get_future作为返回值
    /// </summary>
    /// <typeparam name="F">可调用类型，函数，lambda，function<T>都可以</typeparam>
    /// <typeparam name="...Args">变参模板</typeparam>
    /// <param name="f">可调用类型对象</param>
    /// <param name="...args">可调用类型对象的对应参数列表</param>
    /// <returns></returns>
    template<typename F, typename... Args>
    auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
        using return_type = typename std::invoke_result_t<F, Args...>;
        // 函数体用shared_ptr指向，但后面使用lambda捕获后，把整个lambda转为unique_ptr适配放入队列
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        // 使用 std::function 并捕获 shared_ptr 以避免拷贝问题
        auto wrapped_task = [task]() { (*task)(); };
        task_queue.AddQuestToQueue(std::make_unique<std::function<void()>>(std::move(wrapped_task)));
        return res;
    }
    // 线程池析构
    ~ShabbyThreadPool();

private:
    ShabbyThreadPool(size_t numThreads); // 构造函数私有化
    ShabbyThreadPool(const ShabbyThreadPool&) = delete; // 删除拷贝构造
    ShabbyThreadPool& operator=(const ShabbyThreadPool&) = delete; // 删除赋值构造
    void worker(); // 工作线程函数
    TaskModelQueue<std::function<void()>> task_queue; // 使用事件队列管理任务
    std::vector<std::thread> workers;
    std::atomic<bool> stop;
};
