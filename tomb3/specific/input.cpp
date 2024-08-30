#include "../tomb3/pch.h"
#include "input.h"
#include "dd.h"
#include "di.h"
#include "dxshell.h"
#include "display.h"
#include "../game/invfunc.h"
#include "../game/objects.h"
#include "../game/laramisc.h"
#include "winmain.h"
#include "../game/gameflow.h"
#include "../game/camera.h"
#include "../game/control.h"
#include "../game/lara.h"
#include "picture.h"
#include "smain.h"
#include "hwrender.h"
#include "../tomb3/tomb3.h"

const char* KeyboardButtons[272] =
{
	0,
	"ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "+", "BKSP",
	"TAB", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "<", ">", "RET",
	"CTRL", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "`",
	"SHIFT", "#", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "SHIFT",
	"PADx", "ALT", "SPACE", "CAPS", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"NMLK", 0,
	"PAD7", "PAD8", "PAD9", "PAD-",
	"PAD4", "PAD5", "PAD6", "PAD+",
	"PAD1", "PAD2", "PAD3",
	"PAD0", "PAD.", 0, 0, "\\", 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"ENTER", "CTRL", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"SHIFT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"PAD/", 0, 0, "ALT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"HOME", "UP", "PGUP", 0, "LEFT", 0, "RIGHT", 0, "END", "DOWN", "PGDN", "INS", "DEL",
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	"JOY1", "JOY2", "JOY3", "JOY4", "JOY5", "JOY6", "JOY7", "JOY8",
	"JOY9", "JOY10", "JOY11", "JOY12", "JOY13", "JOY14", "JOY15", "JOY16"
};

short layout[2][NLAYOUTKEYS] =
{
	{DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT, DIK_PERIOD, DIK_SLASH, DIK_RSHIFT,
	DIK_RMENU, DIK_RCONTROL, DIK_SPACE, DIK_COMMA, DIK_NUMPAD0, DIK_END, DIK_ESCAPE, DIK_P,},

	{DIK_NUMPAD8, DIK_NUMPAD2, DIK_NUMPAD4, DIK_NUMPAD6, DIK_DECIMAL, DIK_NUMPAD1, DIK_RSHIFT,
	DIK_RMENU, DIK_RCONTROL, DIK_SPACE, DIK_NUMPAD5, DIK_NUMPAD0, DIK_END, DIK_ESCAPE, DIK_P}
};

long bLaraOn = 1;
long bRoomOn = 1;
long bObjectOn = 1;
long bAObjectOn = 1;
long bEffectOn = 1;
char bInvItemsOff = 0;

long input;
long FinishLevelCheat;
long conflict[15];
uchar keymap[256];

long Key(long number)
{
	short key;

	key = layout[1][number];

	if (key_pressed(key))
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return key_pressed(DIK_LCONTROL);

	case DIK_LCONTROL:
		return key_pressed(DIK_RCONTROL);

	case DIK_RSHIFT:
		return key_pressed(DIK_LSHIFT);

	case DIK_LSHIFT:
		return key_pressed(DIK_RSHIFT);

	case DIK_RMENU:
		return key_pressed(DIK_LMENU);

	case DIK_LMENU:
		return key_pressed(DIK_RMENU);
	}

	if (conflict[number])
		return 0;

	key = layout[0][number];

	if (key_pressed(key))
		return 1;

	switch (key)
	{
	case DIK_RCONTROL:
		return key_pressed(DIK_LCONTROL);

	case DIK_LCONTROL:
		return key_pressed(DIK_RCONTROL);

	case DIK_RSHIFT:
		return key_pressed(DIK_LSHIFT);

	case DIK_LSHIFT:
		return key_pressed(DIK_RSHIFT);

	case DIK_RMENU:
		return key_pressed(DIK_LMENU);

	case DIK_LMENU:
		return key_pressed(DIK_RMENU);
	}

	return 0;
}

bool xinput_devices[4];
gamepad_t gamepads[4];

static void gamepad_update_key(gamepad_t* pad, uint8_t button_name, uint32_t pressed) {

	uint32_t was_pressed = pad->buttons[button_name] & gamepad_key_flag_pressed;
	uint8_t result = pressed ? gamepad_key_flag_pressed : gamepad_key_flag_none;
	result |= (pressed && !was_pressed) ? gamepad_key_flag_just_pressed : gamepad_key_flag_none;
	result |= (!pressed && was_pressed) ? gamepad_key_flag_just_released : gamepad_key_flag_none;
	pad->buttons[button_name] = result;
}

static uint32_t gamepad_is_pressed(gamepad_t* pad, gamepad_button_t button_name) {
	uint32_t result = pad && (pad->buttons[button_name] & gamepad_key_flag_pressed);
	return result;
}

static uint32_t gamepad_just_pressed(gamepad_t* pad, gamepad_button_t button_name) {
	uint32_t result = pad && (pad->buttons[button_name] & gamepad_key_flag_just_pressed);
	return result;
}

static uint32_t gamepad_just_released(gamepad_t* pad, gamepad_button_t button_name) {
	uint32_t result = pad && (pad->buttons[button_name] & gamepad_key_flag_just_released);
	return result;
}

static bool xinput_update() {

	bool result = true;
	bool check_for_new_controller = false;

	static uint64_t frequency = 0;
	static uint64_t previous_time = 0;

	if (!frequency) {
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	}

	uint64_t time;
	QueryPerformanceCounter((LARGE_INTEGER*)&time);

	if (previous_time == 0) {
		check_for_new_controller = true;
		previous_time = time;
	} else {

		float delta = (float)((double)(time - previous_time) / (double)frequency);

		if (delta >= 3.0f) {
			check_for_new_controller = true;
			previous_time = time;
		}
	}

	for (uint32_t i = 0; i < 4; i++) {

		if (xinput_devices[i] || check_for_new_controller) {

			XINPUT_STATE pad_state;
			DWORD result = XInputGetState(i, &pad_state);

			if (result == ERROR_SUCCESS) {

				xinput_devices[i] = true;

				gamepad_t* pad = gamepads + i;
				gamepad_update_key(pad, gamepad_button_menu, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_START);
				gamepad_update_key(pad, gamepad_button_view, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
				gamepad_update_key(pad, gamepad_button_a, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_A);
				gamepad_update_key(pad, gamepad_button_b, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_B);
				gamepad_update_key(pad, gamepad_button_x, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_X);
				gamepad_update_key(pad, gamepad_button_y, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
				gamepad_update_key(pad, gamepad_button_dpad_up, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
				gamepad_update_key(pad, gamepad_button_dpad_down, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
				gamepad_update_key(pad, gamepad_button_dpad_left, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
				gamepad_update_key(pad, gamepad_button_dpad_right, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
				gamepad_update_key(pad, gamepad_button_left_shoulder, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
				gamepad_update_key(pad, gamepad_button_right_shoulder, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
				gamepad_update_key(pad, gamepad_button_left_thumbstick, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
				gamepad_update_key(pad, gamepad_button_right_thumbstick, pad_state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

				pad->left_trigger = pad_state.Gamepad.bLeftTrigger / 255.0f;
				pad->right_trigger = pad_state.Gamepad.bRightTrigger / 255.0f;

				{
					float x = ((float)pad_state.Gamepad.sThumbLX) + 32768;
					float y = ((float)pad_state.Gamepad.sThumbLY) + 32768;
					x = (x / 65535) * 2 - 1;
					y = (y / 65535) * 2 - 1;
					float magnitude = sqrtf((x * x) + (y * y));

					if (magnitude) {
						x /= magnitude;
						y /= magnitude;
						magnitude = (magnitude < 0) ? 0 : ((magnitude > 1) ? 1 : magnitude);
						pad->left_thumbstick_x = x * magnitude;
						pad->left_thumbstick_y = y * magnitude;
					}
					else {
						pad->left_thumbstick_x = 0;
						pad->left_thumbstick_y = 0;
					}
				}

				{
					float x = ((float)pad_state.Gamepad.sThumbRX) + 32768;
					float y = ((float)pad_state.Gamepad.sThumbRY) + 32768;
					x = (x / 65535) * 2 - 1;
					y = (y / 65535) * 2 - 1;
					float magnitude = sqrtf((x * x) + (y * y));

					if (magnitude) {
						x /= magnitude;
						y /= magnitude;
						magnitude = (magnitude < 0) ? 0 : ((magnitude > 1) ? 1 : magnitude);
						pad->right_thumbstick_x = x * magnitude;
						pad->right_thumbstick_y = y * magnitude;
					}
					else {
						pad->right_thumbstick_x = 0;
						pad->right_thumbstick_y = 0;
					}
				}

			} else if (result == ERROR_DEVICE_NOT_CONNECTED) {

				xinput_devices[i] = false;
				gamepad_t* pad = gamepads + i;
				memset(pad, 0, sizeof(gamepad_t));

			} else {
				
				xinput_devices[i] = false;
				gamepad_t* pad = gamepads + i;
				memset(pad, 0, sizeof(gamepad_t));
				result = false;
			}
		}
	}

	return result;
}

long S_UpdateInput()
{
	long linput;
	static long med_debounce = 0;
	static bool pause_debounce = 0;
#if (DIRECT3D_VERSION >= 0x900)
	static bool F7_debounce = 0;
#endif

	DD_SpinMessageLoop(0);
	DI_ReadKeyboard(keymap);

	xinput_update();

	gamepad_t* pad = 0;

	for (uint32_t i = 0; i < 4; i++) {
		if (xinput_devices[i] == true) {
			pad = gamepads + i;
			break;
		}
	}

	linput = 0;

	if (Key(0))
		linput |= IN_FORWARD;

	if (pad && ( gamepad_is_pressed(pad, gamepad_button_dpad_up) || pad->left_thumbstick_y > 0.25f ) ) {
		linput |= IN_FORWARD;
	}

	if (Key(1))
		linput |= IN_BACK;

	if (pad && ( gamepad_is_pressed(pad, gamepad_button_dpad_down) || pad->left_thumbstick_y < -0.25f ) ) {
		linput |= IN_BACK;
	}

	if (Key(2))
		linput |= IN_LEFT;

	if (pad && ( gamepad_is_pressed(pad, gamepad_button_dpad_left) || pad->left_thumbstick_x < -0.25f ) ) {
		linput |= IN_LEFT;
	}

	if (Key(3))
		linput |= IN_RIGHT;

	if (pad && ( gamepad_is_pressed(pad, gamepad_button_dpad_right) || pad->left_thumbstick_x > 0.25f ) ) {
		linput |= IN_RIGHT;
	}

	if (Key(4))
		linput |= IN_DUCK;

	if (pad && pad->left_trigger > 0.25f) {
		linput |= IN_DUCK;
	}

	if (Key(5))
		linput |= IN_SPRINT;

	if (pad && pad->right_trigger > 0.25f) {
		linput |= IN_SPRINT;
	}

	if (Key(6))
		linput |= IN_WALK;

	if (pad && gamepad_is_pressed(pad, gamepad_button_right_shoulder)) {
		linput |= IN_WALK;
	}

	if (Key(7))
		linput |= IN_JUMP;

	if (pad && gamepad_is_pressed(pad, gamepad_button_x)) {
		linput |= IN_JUMP;
	}

	if (Key(8))
		linput |= IN_ACTION;

	if (pad && gamepad_is_pressed(pad, gamepad_button_a)) {
		linput |= IN_ACTION;
	}

	if (Key(9))
		linput |= IN_DRAW;

	if (pad && gamepad_is_pressed(pad, gamepad_button_y)) {
		linput |= IN_DRAW;
	}

	if (Key(10))
		linput |= IN_FLARE;

	if (pad && gamepad_is_pressed( pad, gamepad_button_right_thumbstick)) {
		linput |= IN_FLARE;
	}

	if (Key(11))
		linput |= IN_LOOK;

	if (pad && gamepad_is_pressed(pad, gamepad_button_left_shoulder)) {
		linput |= IN_LOOK;
	}

	if (Key(12))
		linput |= IN_ROLL;

	if (pad && gamepad_is_pressed(pad, gamepad_button_b)) {
		linput |= IN_ROLL;
	}

	bool pause = Key(14) || ( pad && gamepad_is_pressed(pad, gamepad_button_menu) );

	if (pause && !pictureFading)
	{
		if (!pause_debounce)
		{
			pause_debounce = 1;
			linput |= IN_PAUSE;
		}
	}
	else
		pause_debounce = 0;

	if (linput & IN_WALK && !(linput & (IN_FORWARD | IN_BACK)))
	{
		if (linput & IN_LEFT)
			linput = (linput & ~IN_LEFT) | IN_LSTEP;
		else if (linput & IN_RIGHT)
			linput = (linput & ~IN_RIGHT) | IN_RSTEP;
	}

	if (key_pressed(DIK_MULTIPLY))
	{
		farz -= 50;

		if (farz < 0x2000)
			farz = 0x2000;

		distanceFogValue = farz - 0x2000;
	}

	if (key_pressed(DIK_DIVIDE))
	{
		farz += 50;

		if (farz > 0x5000)
			farz = 0x5000;

		distanceFogValue = farz - 0x2000;
	}

	bool option = Key(13) || ( pad && gamepad_is_pressed(pad, gamepad_button_view) );

	if (option && camera.type != CINEMATIC_CAMERA && !pictureFading)
		linput |= IN_OPTION;

	if (linput & IN_FORWARD && linput & IN_BACK)
		linput |= IN_ROLL;

	if (key_pressed(DIK_RETURN) || linput & IN_ACTION)
		linput |= IN_SELECT;

	if (key_pressed(DIK_ESCAPE) || ( pad && gamepad_is_pressed( pad, gamepad_button_view)) )
		linput |= IN_DESELECT;

	if ((linput & (IN_RIGHT | IN_LEFT)) == (IN_RIGHT | IN_LEFT))
		linput -= IN_RIGHT | IN_LEFT;

	if (GnGameMode == GAMEMODE_IN_GAME && !nLoadedPictures)
	{
		if (key_pressed(DIK_ADD))
			IncreaseScreenSize();

		if (key_pressed(DIK_SUBTRACT))
			DecreaseScreenSize();
	}
	
#if (DIRECT3D_VERSION >= 0x900)
	if (key_pressed(DIK_F7))
	{
		if (!F7_debounce)
		{
			F7_debounce = 1;
			tomb3.psx_contrast = !tomb3.psx_contrast;
			HWR_InitState();
			S_SaveSettings();
		}
	}
	else
		F7_debounce = 0;
#endif

	if (key_pressed(DIK_1) && Inv_RequestItem(GUN_OPTION))
		lara.request_gun_type = LG_PISTOLS;
	else if (key_pressed(DIK_2) && Inv_RequestItem(SHOTGUN_OPTION))
		lara.request_gun_type = LG_SHOTGUN;
	else if (key_pressed(DIK_3) && Inv_RequestItem(MAGNUM_OPTION))
		lara.request_gun_type = LG_MAGNUMS;
	else if (key_pressed(DIK_4) && Inv_RequestItem(UZI_OPTION))
		lara.request_gun_type = LG_UZIS;
	else if (key_pressed(DIK_5) && Inv_RequestItem(HARPOON_OPTION))
		lara.request_gun_type = LG_HARPOON;
	else if (key_pressed(DIK_6) && Inv_RequestItem(M16_OPTION))
		lara.request_gun_type = LG_M16;
	else if (key_pressed(DIK_7) && Inv_RequestItem(ROCKET_OPTION))
		lara.request_gun_type = LG_ROCKET;
	else if (key_pressed(DIK_8) && Inv_RequestItem(GRENADE_OPTION))
		lara.request_gun_type = LG_GRENADE;

	if (key_pressed(DIK_0) && Inv_RequestItem(MEDI_OPTION))
	{
		if (!med_debounce)
		{
			UseItem(MEDI_OPTION);
			med_debounce = 15;
		}
	}
	else if (key_pressed(DIK_9) && Inv_RequestItem(BIGMEDI_OPTION))
	{
		if (!med_debounce)
		{
			UseItem(BIGMEDI_OPTION);
			med_debounce = 15;
		}
	}
	else if (med_debounce)
		med_debounce--;

#if (DIRECT3D_VERSION < 0x900)
	if (key_pressed(DIK_APOSTROPHE))
		DXSaveScreen(App.BackBuffer);
#endif

	if (FinishLevelCheat)
	{
		level_complete = 1;
		FinishLevelCheat = 0;
	}

	if (!gameflow.loadsave_disabled && !pictureFading)
	{
		if (key_pressed(DIK_F5))
		{
			if (!tomb3.psx_saving)
				linput |= IN_SAVE;
		}
		else if (key_pressed(DIK_F6))
			linput |= IN_LOAD;
	}

	input = linput;
	return GtWindowClosed;
}
