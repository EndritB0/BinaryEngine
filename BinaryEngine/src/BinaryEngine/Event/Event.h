#pragma once

#include <string>
#include <functional>
#include <type_traits>

namespace BinaryEngine {

	enum class EventType {
		None = 0,
		WindowClosed, WindowResized, WindowFocusedGained, WindowFocusedLost, WindowMinimized, WindowMaximized, WindowRestored,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		TextInput,
	};

	class Event {
	public:
		Event() = default;
		virtual ~Event() = default;

		Event(const Event&) = delete;
		Event& operator=(const Event&) = delete;
		Event(Event&&) = default;
		Event& operator=(Event&&) = default;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual std::string ToString() const { return GetName(); }
	public:
		bool handled{ false };
	};


	class EventDispatcher {
		template<typename T>
		using EventFunction = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event) : m_Event(event) {}

		template<typename T>
			requires(std::is_base_of_v<Event, T>)
		bool Dispatch(EventFunction<T> function)
		{
			if (m_Event.GetEventType() == T::GetStaticType() && !m_Event.handled)
			{
				m_Event.handled = function(*(T*)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

}