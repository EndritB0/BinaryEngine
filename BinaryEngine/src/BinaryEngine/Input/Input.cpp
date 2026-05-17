#include "pch.h"
#include "BinaryEngine/Input/Input.h"

namespace BinaryEngine {

	bool Input::IsKeyPressed(KeyCode key)
	{
		SDL_Scancode scancode{ SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key), nullptr) };
		return IsScanCodePressed(static_cast<ScanCode>(scancode));
	}

	bool Input::IsScanCodePressed(ScanCode scanCode)
	{
		int numKeys{};
		const auto* state{ SDL_GetKeyboardState(&numKeys) };

		if (static_cast<int>(scanCode) < numKeys)
		{
			return state[static_cast<int>(scanCode)];
		}
		return false;
	}

	bool Input::IsModifierActive(KeyModifier modifier)
	{
		SDL_Keymod modState{ SDL_GetModState() };
		return (modState & modifier) == modifier;
	}

	bool Input::IsExactModifierActive(KeyModifier modifier)
	{
		SDL_Keymod modState{ SDL_GetModState() };

		KeyModifier ignoredMods = BinaryEngine::Modifier::NumLock |
			BinaryEngine::Modifier::CapsLock |
			BinaryEngine::Modifier::ScrollLock;

		KeyModifier cleanedModState{ static_cast<KeyModifier>(modState & ~ignoredMods) };
		return cleanedModState == modifier;
	}

	bool Input::IsKeyWithModifierPressed(KeyCode key, KeyModifier modifier)
	{
		return IsKeyPressed(key) && IsModifierActive(modifier);
	}

	bool Input::IsKeyWithExactModifierPressed(KeyCode key, KeyModifier modifier)
	{
		return IsKeyPressed(key) && IsExactModifierActive(modifier);
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		SDL_MouseButtonFlags state{ SDL_GetMouseState(nullptr, nullptr) };
		return (state & SDL_BUTTON_MASK(button)) != 0;
	}

	Vector2f Input::GetMousePosition()
	{
		float x{};
		float y{};
		SDL_GetMouseState(&x, &y);
		return { x, y };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}
}