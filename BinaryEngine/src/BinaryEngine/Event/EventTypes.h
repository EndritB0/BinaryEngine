#pragma once

#include <format>

#include "BinaryEngine/Event/MouseCodes.h"
#include "BinaryEngine/Event/KeyCodes.h"
#include "BinaryEngine/Event/ScanCodes.h"
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

	class MouseButtonEvent : public Event {
	public:
		MouseCode GetButton() const { return m_Button; }
		Vector2f GetPosition() const { return m_Position; }
		float GetX() const { return m_Position.x; }
		float GetY() const { return m_Position.y; }

	protected:
		MouseButtonEvent(MouseCode button, Vector2f position) :
			m_Button(button), m_Position(position)
		{
		}

	protected:
		Vector2f m_Position;
		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent {
	public:
		MouseButtonPressedEvent(MouseCode button, Vector2f position) :
			MouseButtonEvent(button, position)
		{
		}

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "MouseButtonPressed"; };
		virtual std::string ToString() const override
		{
			return std::format("{}: Button {} at ({}, {})", GetName(), m_Button, m_Position.x, m_Position.y);
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		MouseButtonReleasedEvent(MouseCode button, Vector2f position) :
			MouseButtonEvent(button, position)
		{
		}

		static EventType GetStaticType() { return EventType::MouseButtonReleased; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "MouseButtonReleased"; };
		virtual std::string ToString() const override
		{
			return std::format("{}: Button {} at ({}, {})", GetName(), m_Button, m_Position.x, m_Position.y);
		}
	};

	class KeyEvent : public Event {
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }
		ScanCode GetScanCode() const { return m_ScanCode; }
		KeyModifier GetModifiers() const { return m_Modifiers; }

	protected:
		KeyEvent(KeyCode key, ScanCode scan, KeyModifier modifiers) : m_KeyCode(key), m_ScanCode(scan), m_Modifiers(modifiers) {}

	protected:
		KeyCode m_KeyCode;
		ScanCode m_ScanCode;
		KeyModifier m_Modifiers;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(KeyCode key, ScanCode scan, KeyModifier modifiers, bool repeat) :KeyEvent(key, scan, modifiers), m_Repeat(repeat) {}

		bool IsRepeat() const { return m_Repeat; }
		static EventType GetStaticType() { return EventType::KeyPressed; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "KeyPressed"; };
		virtual std::string ToString() const override
		{
			return std::format("{}: KeyCode {}, Scancode {}, Modifiers: {}, Repeat = {}", GetName(), GetKeyName(m_KeyCode), GetScanCodeName(m_ScanCode), GetKeyModifierName(m_Modifiers), m_Repeat);
		}

	private:
		bool m_Repeat{ false };
	};

	class KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(KeyCode key, ScanCode scan, KeyModifier modifiers) : KeyEvent(key, scan, modifiers) {}

		static EventType GetStaticType() { return EventType::KeyReleased; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "KeyReleased"; };
		virtual std::string ToString() const override
		{
			return std::format("{}: KeyCode {}, Scancode {}, Modifiers: {}", GetName(), GetKeyName(m_KeyCode), GetScanCodeName(m_ScanCode), GetKeyModifierName(m_Modifiers));
		}
	};

	class TextInputEvent : public Event {
	public:
		TextInputEvent(const std::string& text) : m_Text(text) {}

		const std::string& GetText() const { return m_Text; }
		static EventType GetStaticType() { return EventType::TextInput; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual const char* GetName() const override { return "TextInput"; };
		virtual std::string ToString() const override
		{
			return std::format("{}: \"{}\"", GetName(), m_Text);
		}

	private:
		std::string m_Text;
	};

}