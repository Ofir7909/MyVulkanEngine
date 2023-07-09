#pragma once

#include "EventCallback.h"
#include "Log.h"

namespace MVE
{
template<typename... Param>
class Event
{
	using IEventCallbackPtr = IEventCallback<Param...>*;

  public:
	Event()	 = default;
	~Event() = default;

	void AddListener(IEventCallbackPtr callback)
	{
		auto pos =
			std::ranges::find_if(callbacks, [callback](IEventCallbackPtr other) { return (*callback) == other; });

		if (pos != callbacks.end()) {
			MVE_WARN("Callback already exist in list.");
			return;
		}
		callbacks.push_back(callback);
	}

	void RemoveListener(IEventCallbackPtr callback)
	{
		auto pos =
			std::ranges::find_if(callbacks, [callback](IEventCallbackPtr other) { return (*callback) == other; });

		if (pos == callbacks.end()) {
			MVE_WARN("Callback doesn't exist in list.");
			return;
		}

		callbacks.erase(pos);
	}

	void Call(const Param&... param)
	{
		for (auto& c : callbacks) { (*c)(param...); }
	}

	void operator+=(IEventCallbackPtr callback) { AddListener(callback); }
	void operator-=(IEventCallbackPtr callback) { RemoveListener(callback); }

	void operator()(const Param&... param) { Call(param...); }

  private:
	using CallbackList = std::list<IEventCallbackPtr>;
	CallbackList callbacks;
};
} // namespace MVE