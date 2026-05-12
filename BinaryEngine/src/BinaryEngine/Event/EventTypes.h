#pragma once

#include <format>

#include "BinaryEngine/Event/Event.h"
#include "BinaryEngine/Core/Math.h"

namespace BinaryEngine {

	class WindowClosedEvent : public Event {
	public:
		WindowClosedEvent() = default;
		static EventType GetStaticType() { return EventType::WindowClosed; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "WindowClosed"; };
	};

	class WindowResizedEvent : public Event {
	public:
		WindowResizedEvent(Vector2i size) : m_Size(size) {}

		const Vector2i& GetSize() const { return m_Size; }
		int GetWidth() const { return m_Size.x; }
		int GetHeight() const { return m_Size.y; }
		static EventType GetStaticType() { return EventType::WindowResized; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "WindowResized"; };
		virtual std::string ToString() const override
		{
			return std::format("{}: {}x{}", GetName(), m_Size.x, m_Size.y);
		}
	private:
		Vector2i m_Size{};
	};

}