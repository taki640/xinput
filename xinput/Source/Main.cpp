#include <iostream>
#include <Windows.h>
#include <Xinput.h>
#include <thread>

static const char* IsButtonPressed(const XINPUT_STATE& state, WORD button)
{
	return state.Gamepad.wButtons & button ? "true " : "false";
}

static BYTE ApplyTriggerThreshold(BYTE rawValue)
{
	return rawValue > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ? rawValue : 0;
}

static float NormalizedTrigger(BYTE rawValue)
{
	return ApplyTriggerThreshold(rawValue) / 255.0f;
}

static SHORT ApplyRightThumbstickDeadZone(SHORT rawValue)
{
	if (rawValue > 0.0f)
		return rawValue > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? rawValue : 0;
	return rawValue < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ? rawValue : 0;
}

static void ApplyThumbstickDeadZone(SHORT rawValueX, SHORT rawValueY, SHORT* outValueX, SHORT* outValueY, SHORT deadZone)
{
	if (rawValueX > 0.0f)
		*outValueX = rawValueX > deadZone ? rawValueX : 0;
	else
		*outValueX = rawValueX < -deadZone ? rawValueX : 0;

	if (rawValueY > 0.0f)
		*outValueY = rawValueY > deadZone ? rawValueY : 0;
	else
		*outValueY = rawValueY < -deadZone ? rawValueY : 0;
}

static void NormalizedThumbstick(SHORT rawValueX, SHORT rawValueY, float* outValueX, float* outValueY, SHORT deadZone)
{
	SHORT deadZoneX;
	SHORT deadZoneY;
	ApplyThumbstickDeadZone(rawValueX, rawValueY, &deadZoneX, &deadZoneY, deadZone);
	*outValueX = deadZoneX / (deadZoneX < 0.0f ? 32768.0f : 32767.0f);
	*outValueY = deadZoneY / (deadZoneY < 0.0f ? 32768.0f : 32767.0f);
}

#define LINE_COUNT "35"

static void ClearConsole()
{
	std::cout << "\033[" LINE_COUNT "A";
}

static void GoToEndConsole()
{
	std::cout << "\033[" LINE_COUNT "B\n";
}

int main()
{
	XINPUT_CAPABILITIES capabilities;
	ZeroMemory(&capabilities, sizeof(XINPUT_CAPABILITIES));
	DWORD result = XInputGetCapabilities(0, 0, &capabilities);
	if (result == ERROR_SUCCESS)
	{
		std::cout << "Controller connected\n";

		if (capabilities.Type != XINPUT_DEVTYPE_GAMEPAD || capabilities.SubType != XINPUT_DEVSUBTYPE_GAMEPAD)
		{
			std::cout << "Input device is not a gamepad\n";
			return 0;
		}

		// Returns false even in a Xbox Series X/S controller, not sure why
		// std::cout << "Supports rumble: " << ((capabilities.Flags & XINPUT_CAPS_FFB_SUPPORTED) != 0) << "\n";
	}
	else if (result == ERROR_DEVICE_NOT_CONNECTED)
	{
		std::cout << "Controller not connected\n";
		return 0;
	}
	else
	{
		std::cout << "Failed to get input device capabilities\n";
		return 1;
	}

	XINPUT_STATE state;
	ZeroMemory(&state, sizeof(XINPUT_STATE));

	bool testVibrationLeft = false;
	bool testVibrationRight = false;

	while (true)
	{
		result = XInputGetState(0, &state);
		if (result == ERROR_DEVICE_NOT_CONNECTED)
		{
			GoToEndConsole();
			std::cout << "Controller was disconnected\n";
			break;
		}
		else if (result != ERROR_SUCCESS)
		{
			GoToEndConsole();
			std::cout << "Failed to get gamepad state\n";
			break;
		}

		std::cout << "- Digital Buttons:\n";
		std::cout << "D-pad Up:           " << IsButtonPressed(state, XINPUT_GAMEPAD_DPAD_UP) << "\n";
		std::cout << "D-pad Down:         " << IsButtonPressed(state, XINPUT_GAMEPAD_DPAD_DOWN) << "\n";
		std::cout << "D-pad Left:         " << IsButtonPressed(state, XINPUT_GAMEPAD_DPAD_LEFT) << "\n";
		std::cout << "D-pad Right:        " << IsButtonPressed(state, XINPUT_GAMEPAD_DPAD_RIGHT) << "\n";
		std::cout << "Start:              " << IsButtonPressed(state, XINPUT_GAMEPAD_START) << "\n";
		std::cout << "Back:               " << IsButtonPressed(state, XINPUT_GAMEPAD_BACK) << "\n";
		std::cout << "Left Thumb:         " << IsButtonPressed(state, XINPUT_GAMEPAD_LEFT_THUMB) << "\n";
		std::cout << "Right Thumb:        " << IsButtonPressed(state, XINPUT_GAMEPAD_RIGHT_THUMB) << "\n";
		std::cout << "Left Shoulder:      " << IsButtonPressed(state, XINPUT_GAMEPAD_LEFT_SHOULDER) << "\n";
		std::cout << "Right Shoulder:     " << IsButtonPressed(state, XINPUT_GAMEPAD_RIGHT_SHOULDER) << "\n";
		std::cout << "A:                  " << IsButtonPressed(state, XINPUT_GAMEPAD_A) << "\n";
		std::cout << "B:                  " << IsButtonPressed(state, XINPUT_GAMEPAD_B) << "\n";
		std::cout << "X:                  " << IsButtonPressed(state, XINPUT_GAMEPAD_X) << "\n";
		std::cout << "Y:                  " << IsButtonPressed(state, XINPUT_GAMEPAD_Y) << "\n";

		std::cout << "\n";
		
		#define PADDING "              " // To clear large numbers
		std::cout << "- Analog Buttons:\n";
		std::cout << "Raw:\n";
		std::cout << "  Left Trigger:     " << static_cast<unsigned>(state.Gamepad.bLeftTrigger) << PADDING "\n";
		std::cout << "  Right Trigger:    " << static_cast<unsigned>(state.Gamepad.bRightTrigger) << PADDING "\n";
		std::cout << "  Left Thumbstick:  (" << state.Gamepad.sThumbLX << ", " << state.Gamepad.sThumbLY << ")" PADDING "\n";
		std::cout << "  Right Thumbstick: (" << state.Gamepad.sThumbRX << ", " << state.Gamepad.sThumbRY << ")" PADDING "\n";

		std::cout << "Thresholds/dead zones applied:\n";
		BYTE lt = ApplyTriggerThreshold(state.Gamepad.bLeftTrigger);
		BYTE rt = ApplyTriggerThreshold(state.Gamepad.bRightTrigger);
		SHORT thumbLX;
		SHORT thumbLY;
		SHORT thumbRX;
		SHORT thumbRY;
		ApplyThumbstickDeadZone(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, &thumbLX, &thumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		ApplyThumbstickDeadZone(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY, &thumbRX, &thumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		std::cout << "  Left Trigger:     " << static_cast<unsigned>(lt) << PADDING "\n";
		std::cout << "  Right Trigger:    " << static_cast<unsigned>(rt) << PADDING "\n";
		std::cout << "  Left Thumbstick:  (" << thumbLX << ", " << thumbLY << ")" PADDING "\n";
		std::cout << "  Right Thumbstick: (" << thumbRX << ", " << thumbRY << ")" PADDING "\n";

		std::cout << "Normalized:\n";
		float nlt = NormalizedTrigger(state.Gamepad.bLeftTrigger);
		float nrt = NormalizedTrigger(state.Gamepad.bRightTrigger);
		float nThumbLX = 0.0f;
		float nThumbLY = 0.0f;
		float nThumbRX = 0.0f;
		float nThumbRY = 0.0f;
		NormalizedThumbstick(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, &nThumbLX, &nThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		NormalizedThumbstick(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY, &nThumbRX, &nThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		std::cout << "  Left Trigger:     " << nlt << PADDING "\n";
		std::cout << "  Right Trigger:    " << nrt << PADDING "\n";
		std::cout << "  Left Thumbstick:  (" << nThumbLX << ", " << nThumbLY << ")" PADDING "\n";
		std::cout << "  Right Thumbstick: (" << nThumbRX << ", " << nThumbRY << ")" PADDING "\n";

		std::cout << "\n";

		std::cout << "- Vibration\n";
		std::cout << "Vibration Test Active (hold RT/LT): " << (testVibrationLeft || testVibrationRight ? "true " : "false") << "\n";

		ClearConsole();

		// Low-frequency motor
		if (nlt > 0.5f)
		{
			testVibrationLeft = true;
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			//vibration.wLeftMotorSpeed = nThumbLY < 0.0f ? 0 : (WORD)(nThumbLY * 65535);
			vibration.wLeftMotorSpeed = (WORD)((nThumbLY + 1) * 0.5f * 65535);
			XInputSetState(0, &vibration);
		}
		else if (testVibrationLeft)
		{
			testVibrationLeft = false;
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			vibration.wLeftMotorSpeed = 0;
			XInputSetState(0, &vibration);
		}

		// High-frequency motor
		if (nrt > 0.5f)
		{
			testVibrationRight = true;
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			// vibration.wRightMotorSpeed = nThumbRY < 0.0f ? 0 : (WORD)(nThumbRY * 65535);
			vibration.wRightMotorSpeed = (WORD)((nThumbRY + 1) * 0.5f * 65535);
			XInputSetState(0, &vibration);
		}
		else if (testVibrationRight)
		{
			testVibrationRight = false;
			XINPUT_VIBRATION vibration;
			ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
			vibration.wRightMotorSpeed = 0;
			XInputSetState(0, &vibration);
		}

		if (GetKeyState(VK_ESCAPE) & 0x8000)
		{
			GoToEndConsole();
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
	}

	return 0;
}
