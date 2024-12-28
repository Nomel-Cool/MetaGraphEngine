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
		 * @brief ���Э�̷���ֵ
		 * ��Generator�ӻ�ȡ������
		 */
		std::optional<std::reference_wrapper<T>> opt;

		/**
		 * @brief ��ȡһ��Generator���󣬸ö����promise_type����
		 *
		 * @return ModelGenerator
		 */
		ModelGenerator get_return_object()
		{
			return ModelGenerator{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		// ΪЭ�̵�״̬��������ڴ�ʧ��ʱ
		// static ModelGenerator get_return_object_on_allocation_failure() { return ModelGenerator{ nullptr }; }

		/**
		 * @brief Э���Ƿ񴴽�ʱ�ͱ����𣬺�������Ҳ������initial_suspend
		 * std::suspend_always��std::suspend_never�Ǳ�׼�������Ѿ�����õ����ͣ�ǰ�߱�ʾ���ǹ��𣬺��߱�ʾ�Ӳ�����
		 *
		 * @return std::suspend_never Э�̴���������
		 */
		auto initial_suspend() const
		{
			return std::suspend_never{};
		}

		/**
		 * @brief Э�����һ��ִ���Ƿ���𣬺�������Ҳ������final_suspend
		 * ����final_suspend����β�׶εĹ��������Ա�����noexcept
		 *
		 * @return std::suspend_always Э�����һ��ִ��Ҳ������
		 */
		auto final_suspend() const noexcept
		{
			return std::suspend_always{};
		}

		/**
		 * @brief ����yield_value�ӿڣ�����co_yield���ص�ֵ
		 *
		 * @tparam Arg ֵ������
		 * @param arg co_yield����ֵ
		 * @return std::suspend_always ִ������������
		 */
		template <typename Arg>
		auto yield_value(Arg&& arg)
		{
			opt.emplace(std::forward<Arg>(arg));
			return std::suspend_always{};
		}

		/**
		 * @brief ����Э����δ�����쳣���������ֱ�����unhandled_exception
		 *
		 */
		void unhandled_exception()
		{
			std::exit(EXIT_FAILURE);
		}

		/**
		 * @brief ��Э�̽���co_return��û�з���ֵʱ�����øú���
		 * ����һ��return_value(expr)������������Э�̽������з���ֵ�����
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
		other.handle = nullptr; // �����������
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
	ModelGenerator(const ModelGenerator&) = delete; // ��ֹ����
	ModelGenerator& operator=(const ModelGenerator&) = delete; // ��ֹ������ֵ
};

#endif // !CO_MODEL_GENERATOR_H
