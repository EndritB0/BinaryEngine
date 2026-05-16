#pragma once

#include <cstdint>

namespace BinaryEngine {

	using ScanCode = std::uint16_t;

	std::string GetScanCodeName(ScanCode scancode);

	namespace Scan {

		inline constexpr ScanCode Unknown{ 0 };
		inline constexpr ScanCode A{ 4 };
		inline constexpr ScanCode B{ 5 };
		inline constexpr ScanCode C{ 6 };
		inline constexpr ScanCode D{ 7 };
		inline constexpr ScanCode E{ 8 };
		inline constexpr ScanCode F{ 9 };
		inline constexpr ScanCode G{ 10 };
		inline constexpr ScanCode H{ 11 };
		inline constexpr ScanCode I{ 12 };
		inline constexpr ScanCode J{ 13 };
		inline constexpr ScanCode K{ 14 };
		inline constexpr ScanCode L{ 15 };
		inline constexpr ScanCode M{ 16 };
		inline constexpr ScanCode N{ 17 };
		inline constexpr ScanCode O{ 18 };
		inline constexpr ScanCode P{ 19 };
		inline constexpr ScanCode Q{ 20 };
		inline constexpr ScanCode R{ 21 };
		inline constexpr ScanCode S{ 22 };
		inline constexpr ScanCode T{ 23 };
		inline constexpr ScanCode U{ 24 };
		inline constexpr ScanCode V{ 25 };
		inline constexpr ScanCode W{ 26 };
		inline constexpr ScanCode X{ 27 };
		inline constexpr ScanCode Y{ 28 };
		inline constexpr ScanCode Z{ 29 };

		inline constexpr ScanCode Num1{ 30 };
		inline constexpr ScanCode Num2{ 31 };
		inline constexpr ScanCode Num3{ 32 };
		inline constexpr ScanCode Num4{ 33 };
		inline constexpr ScanCode Num5{ 34 };
		inline constexpr ScanCode Num6{ 35 };
		inline constexpr ScanCode Num7{ 36 };
		inline constexpr ScanCode Num8{ 37 };
		inline constexpr ScanCode Num9{ 38 };
		inline constexpr ScanCode Num0{ 39 };

		inline constexpr ScanCode Return{ 40 };
		inline constexpr ScanCode Escape{ 41 };
		inline constexpr ScanCode Backspace{ 42 };
		inline constexpr ScanCode Tab{ 43 };
		inline constexpr ScanCode Space{ 44 };
		inline constexpr ScanCode Minus{ 45 };
		inline constexpr ScanCode Equals{ 46 };
		inline constexpr ScanCode LeftBracket{ 47 };
		inline constexpr ScanCode RightBracket{ 48 };
		inline constexpr ScanCode Backslash{ 49 };
		inline constexpr ScanCode NonUsHash{ 50 };
		inline constexpr ScanCode Semicolon{ 51 };
		inline constexpr ScanCode Apostrophe{ 52 };
		inline constexpr ScanCode Grave{ 53 };
		inline constexpr ScanCode Comma{ 54 };
		inline constexpr ScanCode Period{ 55 };
		inline constexpr ScanCode Slash{ 56 };
		inline constexpr ScanCode CapsLock{ 57 };

		inline constexpr ScanCode F1{ 58 };
		inline constexpr ScanCode F2{ 59 };
		inline constexpr ScanCode F3{ 60 };
		inline constexpr ScanCode F4{ 61 };
		inline constexpr ScanCode F5{ 62 };
		inline constexpr ScanCode F6{ 63 };
		inline constexpr ScanCode F7{ 64 };
		inline constexpr ScanCode F8{ 65 };
		inline constexpr ScanCode F9{ 66 };
		inline constexpr ScanCode F10{ 67 };
		inline constexpr ScanCode F11{ 68 };
		inline constexpr ScanCode F12{ 69 };

		inline constexpr ScanCode F13{ 104 };
		inline constexpr ScanCode F14{ 105 };
		inline constexpr ScanCode F15{ 106 };
		inline constexpr ScanCode F16{ 107 };
		inline constexpr ScanCode F17{ 108 };
		inline constexpr ScanCode F18{ 109 };
		inline constexpr ScanCode F19{ 110 };
		inline constexpr ScanCode F20{ 111 };
		inline constexpr ScanCode F21{ 112 };
		inline constexpr ScanCode F22{ 113 };
		inline constexpr ScanCode F23{ 114 };
		inline constexpr ScanCode F24{ 115 };

		inline constexpr ScanCode PrintScreen{ 70 };
		inline constexpr ScanCode ScrollLock{ 71 };
		inline constexpr ScanCode Pause{ 72 };
		inline constexpr ScanCode Insert{ 73 };
		inline constexpr ScanCode Home{ 74 };
		inline constexpr ScanCode PageUp{ 75 };
		inline constexpr ScanCode Delete{ 76 };
		inline constexpr ScanCode End{ 77 };
		inline constexpr ScanCode PageDown{ 78 };
		inline constexpr ScanCode RightArrow{ 79 };
		inline constexpr ScanCode LeftArrow{ 80 };
		inline constexpr ScanCode DownArrow{ 81 };
		inline constexpr ScanCode UpArrow{ 82 };

		inline constexpr ScanCode NumLockClear{ 83 };
		inline constexpr ScanCode KeypadDivide{ 84 };
		inline constexpr ScanCode KeypadMultiply{ 85 };
		inline constexpr ScanCode KeypadMinus{ 86 };
		inline constexpr ScanCode KeypadPlus{ 87 };
		inline constexpr ScanCode KeypadEnter{ 88 };
		inline constexpr ScanCode Keypad1{ 89 };
		inline constexpr ScanCode Keypad2{ 90 };
		inline constexpr ScanCode Keypad3{ 91 };
		inline constexpr ScanCode Keypad4{ 92 };
		inline constexpr ScanCode Keypad5{ 93 };
		inline constexpr ScanCode Keypad6{ 94 };
		inline constexpr ScanCode Keypad7{ 95 };
		inline constexpr ScanCode Keypad8{ 96 };
		inline constexpr ScanCode Keypad9{ 97 };
		inline constexpr ScanCode Keypad0{ 98 };
		inline constexpr ScanCode KeypadPeriod{ 99 };
		inline constexpr ScanCode KeypadEquals{ 103 };

		inline constexpr ScanCode LeftControl{ 224 };
		inline constexpr ScanCode LeftShift{ 225 };
		inline constexpr ScanCode LeftAlt{ 226 };
		inline constexpr ScanCode LeftGui{ 227 };
		inline constexpr ScanCode RightControl{ 228 };
		inline constexpr ScanCode RightShift{ 229 };
		inline constexpr ScanCode RightAlt{ 230 };
		inline constexpr ScanCode RightGui{ 231 };
		inline constexpr ScanCode Mode{ 257 };

		inline constexpr ScanCode Sleep{ 258 };
		inline constexpr ScanCode Wake{ 259 };
		inline constexpr ScanCode MediaPlay{ 262 };
		inline constexpr ScanCode MediaPause{ 263 };
		inline constexpr ScanCode MediaRecord{ 264 };
		inline constexpr ScanCode MediaFastForward{ 265 };
		inline constexpr ScanCode MediaRewind{ 266 };
		inline constexpr ScanCode MediaNextTrack{ 267 };
		inline constexpr ScanCode MediaPreviousTrack{ 268 };
		inline constexpr ScanCode MediaStop{ 269 };
		inline constexpr ScanCode MediaEject{ 270 };
		inline constexpr ScanCode MediaPlayPause{ 271 };
		inline constexpr ScanCode Mute{ 127 };
		inline constexpr ScanCode VolumeUp{ 128 };
		inline constexpr ScanCode VolumeDown{ 129 };

		inline constexpr ScanCode NonUsBackslash{ 100 };
		inline constexpr ScanCode International1{ 135 };
		inline constexpr ScanCode International2{ 136 };
		inline constexpr ScanCode International3{ 137 };
		inline constexpr ScanCode International4{ 138 };
		inline constexpr ScanCode International5{ 139 };
		inline constexpr ScanCode International6{ 140 };
		inline constexpr ScanCode International7{ 141 };
		inline constexpr ScanCode International8{ 142 };
		inline constexpr ScanCode International9{ 143 };
		inline constexpr ScanCode Lang1{ 144 };
		inline constexpr ScanCode Lang2{ 145 };
		inline constexpr ScanCode Lang3{ 146 };
		inline constexpr ScanCode Lang4{ 147 };
		inline constexpr ScanCode Lang5{ 148 };
		inline constexpr ScanCode Lang6{ 149 };
		inline constexpr ScanCode Lang7{ 150 };
		inline constexpr ScanCode Lang8{ 151 };
		inline constexpr ScanCode Lang9{ 152 };

	}

}