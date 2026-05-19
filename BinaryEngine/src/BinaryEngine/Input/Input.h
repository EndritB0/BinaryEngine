#pragma once 

#include "BinaryEngine/Core/Math.h"
#include "BinaryEngine/Event/KeyCodes.h"
#include "BinaryEngine/Event/MouseCodes.h"
#include "BinaryEngine/Event/ScanCodes.h"

namespace BinaryEngine {

	class Input {
	public:
		Input() = delete;
		~Input() = delete;
		Input(const Input&) = delete;
		Input& operator=(const Input&) = delete;
		Input(Input&&) = delete;
		Input& operator=(Input&&) = delete;

		static bool IsKeyPressed(KeyCode key);
		static bool IsScanCodePressed(ScanCode scanCode);
		static bool IsModifierActive(KeyModifier modifier);
		static bool IsExactModifierActive(KeyModifier modifier);
		static bool IsKeyWithModifierPressed(KeyCode key, KeyModifier modifier);
		static bool IsKeyWithExactModifierPressed(KeyCode key, KeyModifier modifier);
		static bool IsMouseButtonPressed(MouseCode button);
		static Vector2f GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

	};

}