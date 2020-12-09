#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "common.h"

struct io_t {
	s32 sig_quit;
	s32 __placeholder__;
	s32 key;

	s32 win_w, win_h;
};

// INPUT FUNCTIONS
// InputRead : handles input from SDL
s32 InputRead(struct io_t *input);

/* InputReadKeys : Handles the Keyboard */
// s32 InputReadKeys(SDL_Event *event, struct io_t *input);

/* InputReadMouse : Handles the Mouse */
// s32 InputReadMouse(SDL_Event *event, struct io_t *input);

/* InputReadWindow : Handles Window Events */
// s32 InputReadWindow(SDL_Event *event, struct io_t *input);

#endif // INPUT_H

