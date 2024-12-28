#pragma once
#ifndef CO_MODEL_GENERATOR_H
#define CO_MODEL_GENERATOR_H

#include <coroutine>
#include <optional>

template<typename T>
class ModelGenerator
{
public:
	class promise_type
	{
	public:
		/**
		 * @brief 存放协程返回值
		 * 由Generator从获取并返回
		 */
		std::optional<std::reference_wrapper<T>> opt;

		/**
		 * @brief 获取一个Generator对象，该对象从promise_type构造
		 *
		 * @return ModelGenerator
		 */
		ModelGenerator get_return_object()
		{
			return ModelGenerator{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		// 为协程的状态对象分配内存失败时
		// static ModelGenerator get_return_object_on_allocation_failure() { return ModelGenerator{ nullptr }; }

		/**
		 * @brief 协程是否创建时就被挂起，函数名字也必须是initial_suspend
		 * std::suspend_always、std::suspend_never是标准库里面已经定义好的类型，前者表示总是挂起，后者表示从不挂起
		 *
		 * @return std::suspend_never 协程创建即运行
		 */
		auto initial_suspend() const
		{
			return std::suspend_never{};
		}

		/**
		 * @brief 协程最后一次执行是否挂起，函数名字也必须是final_suspend
		 * 由于final_suspend是收尾阶段的工作，所以必须是noexcept
		 *
		 * @return std::suspend_always 协程最后一次执行也被挂起
		 */
		auto final_suspend() const noexcept
		{
			return std::suspend_always{};
		}

		/**
		 * @brief 定制yield_value接口，接收co_yield返回的值
		 *
		 * @tparam Arg 值的类型
		 * @param arg co_yield返回值
		 * @return std::suspend_always 执行完后继续挂起
		 */
		template <typename Arg>
		auto yield_value(Arg&& arg)
		{
			opt.emplace(std::forward<Arg>(arg));
			return std::suspend_always{};
		}

		/**
		 * @brief 处理协程中未捕获异常，函数名字必须是unhandled_exception
		 *
		 */
		void unhandled_exception()
		{
			std::exit(EXIT_FAILURE);
		}

		/**
		 * @brief 当协程结束co_return且没有返回值时，调用该函数
		 * 还有一个return_value(expr)函数，负责处理协程结束且有返回值的情况
		 */
		void return_void()
		{
		}
	};

	ModelGenerator() = default;

	ModelGenerator(std::coroutine_handle<promise_type> h): handle(h) {}

	ModelGenerator(ModelGenerator&& other) noexcept
	{
		if (handle) {
			handle.destroy();
		}
		handle = other.handle;
		other.handle = nullptr; // 避免二次销毁
	}

	~ModelGenerator()
	{
		if (handle && handle.done()) {
			handle.destroy();
		}
	}

	T& GetValue()
	{
		if (handle && !handle.done())
			return handle.promise().opt->get();
		else
			throw std::logic_error("The CoGenerator has done!!!");
	}

	bool Resume()
	{
		if (!handle || handle.done())
			return false;
		handle.resume();
		return !handle.done();
	}

	bool Done()
	{
		if (!handle)
			return false;
		return handle.done();
	}

private:
	std::coroutine_handle<promise_type> handle;
	ModelGenerator(const ModelGenerator&) = delete; // 禁止拷贝
	ModelGenerator& operator=(const ModelGenerator&) = delete; // 禁止拷贝赋值
};

#endif // !CO_MODEL_GENERATOR_H
