// ////////////////////////////////////////////////////////////////////////////
// BRIAN CHRZANOWSKI
// 2020-12-05 02:12:51
//
// Input Functions
// ////////////////////////////////////////////////////////////////////////////

#include <SDL.h>

#include "common.h"

#include "io.h"

/* InputReadKeys : Handles the Keyboard */
s32 InputReadKeys(SDL_Event *event, struct io_t *input);

/* InputReadMouse : Handles the Mouse */
s32 InputReadMouse(SDL_Event *event, struct io_t *input);

/* InputReadWindow : Handles Window Events */
s32 InputReadWindow(SDL_Event *event, struct io_t *input);

// InputRead : handles input from SDL
s32 InputRead(struct io_t *input)
{
	SDL_Event event;

	assert(input);

	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_QUIT:
				return -1;
				break;

			case SDL_MOUSEMOTION:
				InputReadMouse(&event, input);
				break;

			case SDL_WINDOWEVENT:
				InputReadWindow(&event, input);
				break;

			case SDL_KEYUP:
			case SDL_KEYDOWN:
				InputReadKeys(&event, input);
				break;

			default:
				break;
		}
	}

	return 0;
}

/* InputReadKeys : Handles the Keyboard */
s32 InputReadKeys(SDL_Event *event, struct io_t *input)
{
	s32 is_keydown;
	s32 rc;

	assert(event);
	assert(input);

	rc = 0;

	is_keydown = event->type == SDL_KEYDOWN;

	// NOTE (brian)
	//   https://wiki.libsdl.org/SDL_Keycode

	// just the first key
	switch (event->key.keysym.scancode) {
		case SDL_SCANCODE_Q:
		{
			input->sig_quit = 1;
			break;
		}

		case SDL_SCANCODE_W:
		{
			input->key_n = is_keydown;
			break;
		}

		case SDL_SCANCODE_A:
		{
			input->key_w = is_keydown;
			break;
		}

		case SDL_SCANCODE_S:
		{
			input->key_s = is_keydown;
			break;
		}

		case SDL_SCANCODE_D:
		{
			input->key_e = is_keydown;
			break;
		}

		case SDL_SCANCODE_SPACE:
		{
			input->key_a = is_keydown;
			break;
		}

		default:
		{
			break;
		}
	}

	return rc;
}

/* InputReadMouse : Handles the Mouse */
s32 InputReadMouse(SDL_Event *event, struct io_t *input)
{
	assert(event);
	assert(input);

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

