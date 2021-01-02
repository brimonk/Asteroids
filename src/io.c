// ////////////////////////////////////////////////////////////////////////////
// BRIAN CHRZANOWSKI
// 2020-12-05 02:12:51
//
// Input Functions
// ////////////////////////////////////////////////////////////////////////////

#include <SDL.h>

#include "common.h"

#include "io.h"

struct intmap_t {
	s32 from, to;
};

static struct intmap_t sdlkey_to_inputkey[] = {
	// numerics
	{ SDLK_0, INPUT_KEY_0 },
	{ SDLK_1, INPUT_KEY_1 },
	{ SDLK_2, INPUT_KEY_2 },
	{ SDLK_3, INPUT_KEY_3 },
	{ SDLK_4, INPUT_KEY_4 },
	{ SDLK_5, INPUT_KEY_5 },
	{ SDLK_6, INPUT_KEY_6 },
	{ SDLK_7, INPUT_KEY_7 },
	{ SDLK_8, INPUT_KEY_8 },
	{ SDLK_9, INPUT_KEY_9 },

	// a-z keys in qwerty layout
	{ SDLK_q, INPUT_KEY_Q },
	{ SDLK_w, INPUT_KEY_W },
	{ SDLK_e, INPUT_KEY_E },
	{ SDLK_r, INPUT_KEY_R },
	{ SDLK_t, INPUT_KEY_T },
	{ SDLK_y, INPUT_KEY_Y },
	{ SDLK_u, INPUT_KEY_U },
	{ SDLK_i, INPUT_KEY_I },
	{ SDLK_o, INPUT_KEY_O },
	{ SDLK_p, INPUT_KEY_P },
	{ SDLK_a, INPUT_KEY_A },
	{ SDLK_s, INPUT_KEY_S },
	{ SDLK_d, INPUT_KEY_D },
	{ SDLK_f, INPUT_KEY_F },
	{ SDLK_g, INPUT_KEY_G },
	{ SDLK_h, INPUT_KEY_H },
	{ SDLK_j, INPUT_KEY_J },
	{ SDLK_k, INPUT_KEY_K },
	{ SDLK_l, INPUT_KEY_L },
	{ SDLK_z, INPUT_KEY_Z },
	{ SDLK_x, INPUT_KEY_X },
	{ SDLK_c, INPUT_KEY_C },
	{ SDLK_v, INPUT_KEY_V },
	{ SDLK_b, INPUT_KEY_B },
	{ SDLK_n, INPUT_KEY_N },
	{ SDLK_m, INPUT_KEY_M },

	// modifier keys
	{ SDLK_LSHIFT, INPUT_KEY_SHIFT },
	{ SDLK_TAB, INPUT_KEY_TAB },

	{ SDLK_UP,    INPUT_KEY_UARROW },
	{ SDLK_RIGHT, INPUT_KEY_RARROW },
	{ SDLK_DOWN,  INPUT_KEY_DARROW },
	{ SDLK_LEFT,  INPUT_KEY_LARROW },

	// extras
	{ SDLK_ESCAPE, INPUT_KEY_ESC },
	{ SDLK_SPACE,  INPUT_KEY_SPACE },
	{ SDLK_HOME,   INPUT_KEY_HOME },
	{ SDLK_END,    INPUT_KEY_END },
	{ SDLK_LCTRL,  INPUT_KEY_CTRL },

	// mouse
	{ SDL_BUTTON_LEFT, INPUT_MOUSE_LEFT },
	{ SDL_BUTTON_MIDDLE, INPUT_MOUSE_CENTER },
	{ SDL_BUTTON_RIGHT, INPUT_MOUSE_RIGHT },

	// controller
	{ SDL_CONTROLLER_BUTTON_A, INPUT_CTRLLR_A },
	{ SDL_CONTROLLER_BUTTON_B, INPUT_CTRLLR_B },
	{ SDL_CONTROLLER_BUTTON_X, INPUT_CTRLLR_X },
	{ SDL_CONTROLLER_BUTTON_Y, INPUT_CTRLLR_Y },
	{ SDL_CONTROLLER_BUTTON_BACK, INPUT_CTRLLR_BACK },
	{ SDL_CONTROLLER_BUTTON_START, INPUT_CTRLLR_START },
	{ SDL_CONTROLLER_BUTTON_LEFTSTICK, INPUT_CTRLLR_LSTICK },
	{ SDL_CONTROLLER_BUTTON_RIGHTSTICK, INPUT_CTRLLR_RSTICK },
	{ SDL_CONTROLLER_BUTTON_LEFTSHOULDER, INPUT_CTRLLR_LSHOULDER },
	{ SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, INPUT_CTRLLR_RSHOULDER },
	{ SDL_CONTROLLER_BUTTON_DPAD_UP, INPUT_CTRLLR_DPAD_U },
	{ SDL_CONTROLLER_BUTTON_DPAD_DOWN, INPUT_CTRLLR_DPAD_D },
	{ SDL_CONTROLLER_BUTTON_DPAD_LEFT, INPUT_CTRLLR_DPAD_L },
	{ SDL_CONTROLLER_BUTTON_DPAD_RIGHT, INPUT_CTRLLR_DPAD_R },
};

/* InputReadKeys : Handles the Keyboard */
s32 InputReadKeys(SDL_Event *event, struct io_t *input);

/* InputReadMouse : Handles the Mouse */
s32 InputReadMouse(SDL_Event *event, struct io_t *input);

/* InputReadWindow : Handles Window Events */
s32 InputReadWindow(SDL_Event *event, struct io_t *input);

// InputCycleKeyState : cycles the key state to give rising / falling edges
void InputCycleKeyState(struct io_t *io);

// InputRead : handles input from SDL
s32 InputRead(struct io_t *io)
{
	SDL_Event event;

	assert(io);

	InputCycleKeyState(io);

	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_QUIT:
				return -1;
				break;

			case SDL_MOUSEMOTION:
				InputReadMouse(&event, io);
				break;

			case SDL_WINDOWEVENT:
				InputReadWindow(&event, io);
				break;

			case SDL_KEYUP:
			case SDL_KEYDOWN:
				InputReadKeys(&event, io);
				break;

			default:
				break;
		}
	}

	return 0;
}

/* InputReadKeys : Handles the Keyboard */
s32 InputReadKeys(SDL_Event *event, struct io_t *io)
{
	s32 start, end, bval, bstate, i;

	switch (event->type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			start = INPUT_KEY__START;
			end = INPUT_KEY__END;
			bval = event->key.keysym.sym;
			bstate = event->key.state;
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			start = INPUT_KEY__START;
			end = INPUT_KEY__START;
			bval = event->button.button;
			bstate = event->button.state;
			break;

		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			start = INPUT_CTRLLR__START;
			end = INPUT_CTRLLR__END;
			bval = event->cbutton.button;
			bstate = event->cbutton.state;
			break;

		default:
			LOG("Bad event type [%d] for %s", event->type, __FUNCTION__);
			return -1;
	}

	for (i = start; i < end; i++) {
		if (bval == sdlkey_to_inputkey[i].from) {
			if (bstate == SDL_PRESSED) {
				io->keys[sdlkey_to_inputkey[i].to] = INSTATE_PRESSED;
			} else {
				io->keys[sdlkey_to_inputkey[i].to] = INSTATE_RELEASED;
			}
		}
	}

	return 0;
}

// InputCycleKeyState : cycles the key state to give rising / falling edges
void InputCycleKeyState(struct io_t *io)
{
	s32 i;

	for (i = 0; i < INPUT_KEY_TOTAL; i++) {
		switch (io->keys[i]) {
			case INSTATE_PRESSED:
				io->keys[i] = INSTATE_DOWN;
				break;
			case INSTATE_RELEASED:
				io->keys[i] = INSTATE_UP;
				break;
		}
	}
}

/* InputReadMouse : Handles the Mouse */
s32 InputReadMouse(SDL_Event *event, struct io_t *io)
{
	assert(event);
	assert(io);

	if (event->type != SDL_MOUSEMOTION) {
		return -1;
	}

	// SDL_GetMouseState(&vals->uMouse_x, &vals->uMouse_y);

	return 0;
}

/* InputReadWindow : Handles Window Events */
s32 InputReadWindow(SDL_Event *event, struct io_t *io)
{
	assert(event);
	assert(io);

	if (event->type != SDL_WINDOWEVENT) {
		return -1;
	}

	switch (event->window.event) {
	case SDL_WINDOWEVENT_SHOWN:
		break;
	case SDL_WINDOWEVENT_HIDDEN:
		break;
	case SDL_WINDOWEVENT_EXPOSED:
		break;
	case SDL_WINDOWEVENT_MOVED:
		break;
	case SDL_WINDOWEVENT_RESIZED:
	case SDL_WINDOWEVENT_SIZE_CHANGED:
		io->win_w = event->window.data1;
		io->win_h = event->window.data2;
		break;
	case SDL_WINDOWEVENT_MINIMIZED:
		break;
	case SDL_WINDOWEVENT_MAXIMIZED:
		break;
	case SDL_WINDOWEVENT_RESTORED:
		break;
	case SDL_WINDOWEVENT_ENTER:
		break;
	case SDL_WINDOWEVENT_LEAVE:
		break;
	case SDL_WINDOWEVENT_FOCUS_GAINED:
		break;
	case SDL_WINDOWEVENT_FOCUS_LOST:
		break;
	default:
		SDL_Log("Window %d got unknown event %d", event->window.windowID, event->window.event);
		break;
	}

	return 0;
}

