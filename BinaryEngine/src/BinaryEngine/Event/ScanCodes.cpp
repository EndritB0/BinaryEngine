#include "pch.h"
#include "BinaryEngine/Event/ScanCodes.h"

namespace BinaryEngine {

	std::string GetScanCodeName(ScanCode scancode)
	{
		const char* name{ SDL_GetScancodeName(static_cast<SDL_Scancode>(scancode)) };

		if (!name || name[0] == '\0')
		{
			return "Unknown";
		}
		return name;
	}

}