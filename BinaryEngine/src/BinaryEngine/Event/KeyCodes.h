#pragma once

#include <cstdint>

namespace BinaryEngine {

	using KeyCode = std::uint32_t;
	using KeyModifier = std::uint16_t;

	std::string GetKeyName(KeyCode key);
	std::string GetKeyModifierName(KeyModifier modifier);

	namespace Key {

		inline constexpr KeyCode Unknown{ 0 };

		inline constexpr KeyCode A{ 'a' };
		inline constexpr KeyCode B{ 'b' };
		inline constexpr KeyCode C{ 'c' };
		inline constexpr KeyCode D{ 'd' };
		inline constexpr KeyCode E{ 'e' };
		inline constexpr KeyCode F{ 'f' };
		inline constexpr KeyCode G{ 'g' };
		inline constexpr KeyCode H{ 'h' };
		inline constexpr KeyCode I{ 'i' };
		inline constexpr KeyCode J{ 'j' };
		inline constexpr KeyCode K{ 'k' };
		inline constexpr KeyCode L{ 'l' };
		inline constexpr KeyCode M{ 'm' };
		inline constexpr KeyCode N{ 'n' };
		inline constexpr KeyCode O{ 'o' };
		inline constexpr KeyCode P{ 'p' };
		inline constexpr KeyCode Q{ 'q' };
		inline constexpr KeyCode R{ 'r' };
		inline constexpr KeyCode S{ 's' };
		inline constexpr KeyCode T{ 't' };
		inline constexpr KeyCode U{ 'u' };
		inline constexpr KeyCode V{ 'v' };
		inline constexpr KeyCode W{ 'w' };
		inline constexpr KeyCode X{ 'x' };
		inline constexpr KeyCode Y{ 'y' };
		inline constexpr KeyCode Z{ 'z' };

		inline constexpr KeyCode Num0{ '0' };
		inline constexpr KeyCode Num1{ '1' };
		inline constexpr KeyCode Num2{ '2' };
		inline constexpr KeyCode Num3{ '3' };
		inline constexpr KeyCode Num4{ '4' };
		inline constexpr KeyCode Num5{ '5' };
		inline constexpr KeyCode Num6{ '6' };
		inline constexpr KeyCode Num7{ '7' };
		inline constexpr KeyCode Num8{ '8' };
		inline constexpr KeyCode Num9{ '9' };

		inline constexpr KeyCode Return{ '\r' };
		inline constexpr KeyCode Escape{ 0x1B };
		inline constexpr KeyCode Backspace{ '\b' };
		inline constexpr KeyCode Tab{ '\t' };
		inline constexpr KeyCode Space{ ' ' };
		inline constexpr KeyCode Exclaim{ '!' };
		inline constexpr KeyCode Quote{ '"' };
		inline constexpr KeyCode Hash{ '#' };
		inline constexpr KeyCode Dollar{ '$' };
		inline constexpr KeyCode Percent{ '%' };
		inline constexpr KeyCode Ampersand{ '&' };
		inline constexpr KeyCode Apostrophe{ '\'' };
		inline constexpr KeyCode LeftParenthesis{ '(' };
		inline constexpr KeyCode RightParenthesis{ ')' };
		inline constexpr KeyCode Asterisk{ '*' };
		inline constexpr KeyCode Plus{ '+' };
		inline constexpr KeyCode Comma{ ',' };
		inline constexpr KeyCode Minus{ '-' };
		inline constexpr KeyCode Period{ '.' };
		inline constexpr KeyCode Slash{ '/' };
		inline constexpr KeyCode Colon{ ':' };
		inline constexpr KeyCode Semicolon{ ';' };
		inline constexpr KeyCode LessThan{ '<' };
		inline constexpr KeyCode Equal{ '=' };
		inline constexpr KeyCode GreaterThan{ '>' };
		inline constexpr KeyCode Question{ '?' };
		inline constexpr KeyCode At{ '@' };
		inline constexpr KeyCode LeftBracket{ '[' };
		inline constexpr KeyCode Backslash{ '\\' };
		inline constexpr KeyCode RightBracket{ ']' };
		inline constexpr KeyCode Caret{ '^' };
		inline constexpr KeyCode Underscore{ '_' };
		inline constexpr KeyCode Grave{ '`' };
		inline constexpr KeyCode LeftBrace{ '{' };
		inline constexpr KeyCode Pipe{ '|' };
		inline constexpr KeyCode RightBrace{ '}' };
		inline constexpr KeyCode Tilde{ '~' };
		inline constexpr KeyCode Delete{ 0x7F };
		inline constexpr KeyCode CapsLock{ 0x40000039 };

		inline constexpr KeyCode F1{ 0x4000003a };
		inline constexpr KeyCode F2{ 0x4000003b };
		inline constexpr KeyCode F3{ 0x4000003c };
		inline constexpr KeyCode F4{ 0x4000003d };
		inline constexpr KeyCode F5{ 0x4000003e };
		inline constexpr KeyCode F6{ 0x4000003f };
		inline constexpr KeyCode F7{ 0x40000040 };
		inline constexpr KeyCode F8{ 0x40000041 };
		inline constexpr KeyCode F9{ 0x40000042 };
		inline constexpr KeyCode F10{ 0x40000043 };
		inline constexpr KeyCode F11{ 0x40000044 };
		inline constexpr KeyCode F12{ 0x40000045 };

		inline constexpr KeyCode F13{ 0x40000068 };
		inline constexpr KeyCode F14{ 0x40000069 };
		inline constexpr KeyCode F15{ 0x4000006a };
		inline constexpr KeyCode F16{ 0x4000006b };
		inline constexpr KeyCode F17{ 0x4000006c };
		inline constexpr KeyCode F18{ 0x4000006d };
		inline constexpr KeyCode F19{ 0x4000006e };
		inline constexpr KeyCode F20{ 0x4000006f };
		inline constexpr KeyCode F21{ 0x40000070 };
		inline constexpr KeyCode F22{ 0x40000071 };
		inline constexpr KeyCode F23{ 0x40000072 };
		inline constexpr KeyCode F24{ 0x40000073 };

		inline constexpr KeyCode PrintScreen{ 0x40000046 };
		inline constexpr KeyCode ScrollLock{ 0x40000047 };
		inline constexpr KeyCode Pause{ 0x40000048 };
		inline constexpr KeyCode Insert{ 0x40000049 };
		inline constexpr KeyCode Home{ 0x4000004a };
		inline constexpr KeyCode PageUp{ 0x4000004b };
		inline constexpr KeyCode End{ 0x4000004d };
		inline constexpr KeyCode PageDown{ 0x4000004e };
		inline constexpr KeyCode RightArrow{ 0x4000004f };
		inline constexpr KeyCode LeftArrow{ 0x40000050 };
		inline constexpr KeyCode DownArrow{ 0x40000051 };
		inline constexpr KeyCode UpArrow{ 0x40000052 };

		inline constexpr KeyCode NumLockClear{ 0x40000053 };
		inline constexpr KeyCode KeypadDivide{ 0x40000054 };
		inline constexpr KeyCode KeypadMultiply{ 0x40000055 };
		inline constexpr KeyCode KeypadMinus{ 0x40000056 };
		inline constexpr KeyCode KeypadPlus{ 0x40000057 };
		inline constexpr KeyCode KeypadEnter{ 0x40000058 };
		inline constexpr KeyCode Keypad1{ 0x40000059 };
		inline constexpr KeyCode Keypad2{ 0x4000005a };
		inline constexpr KeyCode Keypad3{ 0x4000005b };
		inline constexpr KeyCode Keypad4{ 0x4000005c };
		inline constexpr KeyCode Keypad5{ 0x4000005d };
		inline constexpr KeyCode Keypad6{ 0x4000005e };
		inline constexpr KeyCode Keypad7{ 0x4000005f };
		inline constexpr KeyCode Keypad8{ 0x40000060 };
		inline constexpr KeyCode Keypad9{ 0x40000061 };
		inline constexpr KeyCode Keypad0{ 0x40000062 };
		inline constexpr KeyCode KeypadPeriod{ 0x40000063 };
		inline constexpr KeyCode KeypadEquals{ 0x40000067 };

		inline constexpr KeyCode LeftControl{ 0x400000e0 };
		inline constexpr KeyCode LeftShift{ 0x400000e1 };
		inline constexpr KeyCode LeftAlt{ 0x400000e2 };
		inline constexpr KeyCode LeftGui{ 0x400000e3 };
		inline constexpr KeyCode RightControl{ 0x400000e4 };
		inline constexpr KeyCode RightShift{ 0x400000e5 };
		inline constexpr KeyCode RightAlt{ 0x400000e6 };
		inline constexpr KeyCode RightGui{ 0x400000e7 };
		inline constexpr KeyCode Mode{ 0x40000101 };

		inline constexpr KeyCode Sleep{ 0x40000102 };
		inline constexpr KeyCode Wake{ 0x40000103 };
		inline constexpr KeyCode MediaPlay{ 0x40000106 };
		inline constexpr KeyCode MediaPause{ 0x40000107 };
		inline constexpr KeyCode MediaRecord{ 0x40000108 };
		inline constexpr KeyCode MediaFastForward{ 0x40000109 };
		inline constexpr KeyCode MediaRewind{ 0x4000010a };
		inline constexpr KeyCode MediaNextTrack{ 0x4000010b };
		inline constexpr KeyCode MediaPreviousTrack{ 0x4000010c };
		inline constexpr KeyCode MediaStop{ 0x4000010d };
		inline constexpr KeyCode MediaEject{ 0x4000010e };
		inline constexpr KeyCode MediaPlayPause{ 0x4000010f };
		inline constexpr KeyCode Mute{ 0x4000007f };
		inline constexpr KeyCode VolumeUp{ 0x40000080 };
		inline constexpr KeyCode VolumeDown{ 0x40000081 };

	}

	namespace Modifier {

		inline constexpr KeyModifier None{ 0 };
		inline constexpr KeyModifier LeftShift{ 0x0001 };
		inline constexpr KeyModifier RightShift{ 0x0002 };
		inline constexpr KeyModifier Level5{ 0x0004 };
		inline constexpr KeyModifier LeftCtrl{ 0x0040 };
		inline constexpr KeyModifier RightCtrl{ 0x0080 };
		inline constexpr KeyModifier LeftAlt{ 0x0100 };
		inline constexpr KeyModifier RightAlt{ 0x0200 };
		inline constexpr KeyModifier LeftGui{ 0x0400 };
		inline constexpr KeyModifier RightGui{ 0x0800 };
		inline constexpr KeyModifier NumLock{ 0x1000 };
		inline constexpr KeyModifier CapsLock{ 0x2000 };
		inline constexpr KeyModifier Mode{ 0x4000 };
		inline constexpr KeyModifier ScrollLock{ 0x8000 };
		inline constexpr KeyModifier Ctrl{ (LeftCtrl | RightCtrl) };
		inline constexpr KeyModifier Shift{ (LeftShift | RightShift) };
		inline constexpr KeyModifier Alt{ (LeftAlt | RightAlt) };
		inline constexpr KeyModifier Gui{ (LeftGui | RightGui) };

	}

}