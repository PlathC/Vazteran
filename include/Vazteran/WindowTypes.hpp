#ifndef VAZTERAN_WINDOWTYPES_HPP
#define VAZTERAN_WINDOWTYPES_HPP

#include <type_traits>

#include <GLFW/glfw3.h>

#include "Vazteran/Core/Utils.hpp"

namespace vzt
{
	enum class MouseButton : uint8_t
	{
		Button1 = GLFW_MOUSE_BUTTON_1,
		Button2 = GLFW_MOUSE_BUTTON_2,
		Button3 = GLFW_MOUSE_BUTTON_3,
		Button4 = GLFW_MOUSE_BUTTON_4,
		Button5 = GLFW_MOUSE_BUTTON_5,
		Button6 = GLFW_MOUSE_BUTTON_6,
		Button7 = GLFW_MOUSE_BUTTON_7,
		Button8 = GLFW_MOUSE_BUTTON_8,
		Left    = GLFW_MOUSE_BUTTON_LEFT,
		Right   = GLFW_MOUSE_BUTTON_RIGHT,
		Middle  = GLFW_MOUSE_BUTTON_MIDDLE
	};

	enum class KeyAction : uint8_t
	{
		Release = GLFW_RELEASE,
		Press   = GLFW_PRESS,
		Repeat  = GLFW_REPEAT
	};

	enum class KeyModifier : uint8_t
	{
		Shift    = GLFW_MOD_SHIFT,
		Control  = GLFW_MOD_CONTROL,
		Alt      = GLFW_MOD_ALT,
		Super    = GLFW_MOD_SUPER,
		CapsLock = GLFW_MOD_CAPS_LOCK,
		NumLock  = GLFW_MOD_NUM_LOCK
	};

	inline KeyModifier operator&(KeyModifier l, KeyModifier r)
	{
		return static_cast<KeyModifier>(vzt::ToUnderlying(l) & vzt::ToUnderlying(r));
	}

	inline KeyModifier operator|(KeyModifier l, KeyModifier r)
	{
		return static_cast<KeyModifier>(vzt::ToUnderlying(l) | vzt::ToUnderlying(r));
	}

	inline constexpr KeyModifier operator~(const KeyModifier m)
	{
		return static_cast<KeyModifier>(~vzt::ToUnderlying(m));
	}

	enum class KeyCode : int32_t
	{
		Unknown    = GLFW_KEY_UNKNOWN,
		Space      = GLFW_KEY_SPACE,
		Apostrophe = GLFW_KEY_APOSTROPHE,

		Comma  = GLFW_KEY_COMMA,
		Minus  = GLFW_KEY_MINUS,
		Period = GLFW_KEY_PERIOD,
		Slash  = GLFW_KEY_SLASH,

		Zero  = GLFW_KEY_0,
		One   = GLFW_KEY_1,
		Two   = GLFW_KEY_2,
		Three = GLFW_KEY_3,
		Four  = GLFW_KEY_4,
		Five  = GLFW_KEY_5,
		Six   = GLFW_KEY_6,
		Seven = GLFW_KEY_7,
		Eight = GLFW_KEY_8,
		Nine  = GLFW_KEY_9,

		SemiColon = GLFW_KEY_SEMICOLON,

		Equal = GLFW_KEY_EQUAL,

		A = GLFW_KEY_A,
		B = GLFW_KEY_B,
		C = GLFW_KEY_C,
		D = GLFW_KEY_D,
		E = GLFW_KEY_E,
		F = GLFW_KEY_F,
		G = GLFW_KEY_G,
		H = GLFW_KEY_H,
		I = GLFW_KEY_I,
		J = GLFW_KEY_J,
		K = GLFW_KEY_K,
		L = GLFW_KEY_L,
		M = GLFW_KEY_M,
		N = GLFW_KEY_N,
		O = GLFW_KEY_O,
		P = GLFW_KEY_P,
		Q = GLFW_KEY_Q,
		R = GLFW_KEY_R,
		S = GLFW_KEY_S,
		T = GLFW_KEY_T,
		U = GLFW_KEY_U,
		V = GLFW_KEY_V,
		W = GLFW_KEY_W,
		X = GLFW_KEY_X,
		Y = GLFW_KEY_Y,
		Z = GLFW_KEY_Z,

		LeftBracket  = GLFW_KEY_LEFT_BRACKET,
		BackSlash    = GLFW_KEY_BACKSLASH,
		RightBracket = GLFW_KEY_RIGHT_BRACKET,
		GraveAccent  = GLFW_KEY_GRAVE_ACCENT,
		World1       = GLFW_KEY_WORLD_1,
		World2       = GLFW_KEY_WORLD_2,
		Escape       = GLFW_KEY_ESCAPE,
		Enter        = GLFW_KEY_ENTER,
		Tab          = GLFW_KEY_TAB,
		BackSpace    = GLFW_KEY_BACKSPACE,
		Insert       = GLFW_KEY_INSERT,
		Delete       = GLFW_KEY_DELETE,
		Right        = GLFW_KEY_RIGHT,
		Left         = GLFW_KEY_LEFT,
		Down         = GLFW_KEY_DOWN,
		Up           = GLFW_KEY_UP,
		PageUp       = GLFW_KEY_PAGE_UP,
		PageDown     = GLFW_KEY_PAGE_DOWN,
		Home         = GLFW_KEY_HOME,
		End          = GLFW_KEY_END,
		CapsLock     = GLFW_KEY_CAPS_LOCK,
		ScrollLock   = GLFW_KEY_SCROLL_LOCK,
		NumLock      = GLFW_KEY_NUM_LOCK,
		PrintScreen  = GLFW_KEY_PRINT_SCREEN,
		Pause        = GLFW_KEY_PAUSE,
		F1           = GLFW_KEY_F1,
		F2           = GLFW_KEY_F2,
		F3           = GLFW_KEY_F3,
		F4           = GLFW_KEY_F4,
		F5           = GLFW_KEY_F5,
		F6           = GLFW_KEY_F6,
		F7           = GLFW_KEY_F7,
		F8           = GLFW_KEY_F8,
		F9           = GLFW_KEY_F9,
		F10          = GLFW_KEY_F10,
		F11          = GLFW_KEY_F11,
		F12          = GLFW_KEY_F12,
		F13          = GLFW_KEY_F13,
		F14          = GLFW_KEY_F14,
		F15          = GLFW_KEY_F15,
		F16          = GLFW_KEY_F16,
		F17          = GLFW_KEY_F17,
		F18          = GLFW_KEY_F18,
		F19          = GLFW_KEY_F19,
		F20          = GLFW_KEY_F20,
		F21          = GLFW_KEY_F21,
		F22          = GLFW_KEY_F22,
		F23          = GLFW_KEY_F23,
		F24          = GLFW_KEY_F24,
		F25          = GLFW_KEY_F25,
		KP0          = GLFW_KEY_KP_0,
		KP1          = GLFW_KEY_KP_1,
		KP2          = GLFW_KEY_KP_2,
		KP3          = GLFW_KEY_KP_3,
		KP4          = GLFW_KEY_KP_4,
		KP5          = GLFW_KEY_KP_5,
		KP6          = GLFW_KEY_KP_6,
		KP7          = GLFW_KEY_KP_7,
		KP8          = GLFW_KEY_KP_8,
		KP9          = GLFW_KEY_KP_9,
		KPDecimal    = GLFW_KEY_KP_DECIMAL,
		KPDivide     = GLFW_KEY_KP_DIVIDE,
		KPMultiply   = GLFW_KEY_KP_MULTIPLY,
		KPSubtract   = GLFW_KEY_KP_SUBTRACT,
		KPAdd        = GLFW_KEY_KP_ADD,
		KPEnter      = GLFW_KEY_KP_ENTER,
		KPEgal       = GLFW_KEY_KP_EQUAL,
		LeftShift    = GLFW_KEY_LEFT_SHIFT,
		LeftControl  = GLFW_KEY_LEFT_CONTROL,
		LeftAlt      = GLFW_KEY_LEFT_ALT,
		LeftSuper    = GLFW_KEY_LEFT_SUPER,
		RightShift   = GLFW_KEY_RIGHT_SHIFT,
		RightControl = GLFW_KEY_RIGHT_CONTROL,
		RightAlt     = GLFW_KEY_RIGHT_ALT,
		RightSuper   = GLFW_KEY_RIGHT_SUPER,
		Menu         = GLFW_KEY_MENU,
	};

} // namespace vzt

#endif // VAZTERAN_WINDOWTYPES_HPP