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

	/**
	 * @brief Э�̾�����洢��Э�������ģ�һ���ǳ��ײ�Ķ�����û��RAII
	 * ��MyCoroGenerator����
	 */
	std::coroutine_handle<promise_type> handle;

	/**
	 * @brief Ĭ�Ϲ��캯��
	 *
	 */
	ModelGenerator() = default;

	/**
	 * @brief ͨ��һ��handle����һ��Generator
	 *
	 * @param h ��promise_type���������Э�̾��
	 */
	ModelGenerator(std::coroutine_handle<promise_type> h)
		: handle(h)
	{
	}

	/**
	 * @brief �ƶ����캯��
	 *
	 * @param other ����ModelGenerator����
	 */
	ModelGenerator(ModelGenerator&& other)
	{
		if (handle) {
			handle.destroy();
		}
		handle = other.handle;
		other.handle = nullptr;
	}

	/**
	 * @brief ��������
	 *
	 */
	~ModelGenerator()
	{
		if (handle && handle.done()) {
			handle.destroy();
		}
	}


	ModelGenerator& operator=(ModelGenerator&& other)
	{
		if (handle) {
			handle.destroy();
		}
		handle = other.handle;
		other.handle = nullptr;
		return *this;
	}

	/**
	 * @brief ����ִ�н��
	 *
	 * @return T& ���ص�ֵ
	 */
	T& GetValue()
	{
		return handle.promise().opt->get();
	}

	/**
	 * @brief ����ִ��Э��
	 */
	bool Resume()
	{
		if (handle.done())
		{
			throw std::runtime_error("Generator done");
			return false;
		}
		handle.resume();
		return true;
	}

private:
	ModelGenerator(const ModelGenerator&) = delete;
	ModelGenerator& operator=(const ModelGenerator&) = delete;
};

#endif // !CO_MODEL_GENERATOR_H
