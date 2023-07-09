#pragma once

namespace MVE
{
template<typename... Param>
class IEventCallback
{
  public:
	virtual void operator()(const Param&... param)			 = 0;
	virtual bool operator==(IEventCallback<Param...>* other) = 0;
};

template<typename T, typename... Param>
class MemFuncEventCallback : public IEventCallback<Param...>
{
  public:
	using FuncType = void (T::*)(Param...);

  public:
	MemFuncEventCallback(T* instance, FuncType function): instance(instance), function(function) {}

	void operator()(const Param&... param) override { (instance->*function)(param...); }

	bool operator==(IEventCallback<Param...>* other) override
	{
		auto otherE = dynamic_cast<MemFuncEventCallback<T, Param...>*>(other);
		if (otherE == nullptr)
			return false;

		return instance == otherE->instance && function == otherE->function;
	}

  private:
	T* instance;
	FuncType function;
};

template<typename... Param>
class GlobalFuncEventCallback : public IEventCallback<Param...>
{
  public:
	using FuncType = void (*)(Param...);

  public:
	GlobalFuncEventCallback(FuncType function): function(function) {}

	void operator()(const Param&... param) override { (*function)(param...); }

	bool operator==(IEventCallback<Param...>* other) override
	{
		auto otherE = dynamic_cast<GlobalFuncEventCallback<Param...>*>(other);
		if (otherE == nullptr)
			return false;

		return function == otherE->function;
	}

  private:
	FuncType function;
};
} // namespace MVE