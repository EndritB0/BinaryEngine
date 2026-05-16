#include "pch.h"
#include "BinaryEngine/Event/KeyCodes.h"

namespace BinaryEngine {

	std::string GetKeyName(KeyCode key)
	{
		const char* name{ SDL_GetKeyName(static_cast<SDL_Keycode>(key)) };

		if (!name || name[0] == '\0')
		{
			return "Unknown";
		}
		return name;
	}

	std::string GetKeyModifierName(KeyModifier modifier)
	{
		if (modifier == Modifier::None)
		{
			return "None";
		}

		std::string name;
		name.reserve(64);

		auto appendName = [&name](const char* modifierName) {
			if (!name.empty())
			{
				name += " + ";
			}
			name += modifierName;
			};

		if (modifier & Modifier::LeftShift) { appendName("LShift"); }
		if (modifier & Modifier::RightShift) { appendName("RShift"); }
		if (modifier & Modifier::LeftCtrl) { appendName("LCtrl"); }
		if (modifier & Modifier::RightCtrl) { appendName("RCtrl"); }
		if (modifier & Modifier::LeftAlt) { appendName("LAlt"); }
		if (modifier & Modifier::RightAlt) { appendName("RAlt"); }
		if (modifier & Modifier::LeftGui) { appendName("LGui"); }
		if (modifier & Modifier::RightGui) { appendName("RGui"); }
		if (modifier & Modifier::NumLock) { appendName("NumLock"); }
		if (modifier & Modifier::CapsLock) { appendName("CapsLock"); }
		if (modifier & Modifier::ScrollLock) { appendName("ScrollLock"); }

		return name;
	}

}