#ifndef INPUT_H
#define INPUT_H

#include <SDL.h>

#include "common.h"

// NOTE (Brian) the real thing you'd want is the entire SDL keymap exposed here
// and for another project, I totally had basically created an abstraction
// layer around SDL.
//
// Long story short, it's super icky to have an abstraction layer around an abstraction layer. Like,
// what's even the point of doing it in the first place. Ergo, this IO system is a little custom
// tuned for this asteroids game.
//
// I'm sure it'll be fine, and won't bite me in the butt later.

struct io_t {
	s32 sig_quit;
	s32 __placeholder__;

	// directional keys (north, south, east, west)
	s32 key_n;
	s32 key_s;
	s32 key_e;
	s32 key_w;

	// action keys
	s32 key_a;
	s32 key_b;
	s32 key_x;
	s32 key_y;

	s32 key_r;
	s32 key_l;

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

