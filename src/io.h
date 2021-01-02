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

// KEYSTATE
//
// The keystate is what allows us to take the down + up actions from the operating system (SDL for
// now), and ensure that the game can do things on only the rising / falling edges of a button
// action if we really wanted to
//
// So, for this
//
//          /‾‾‾‾‾‾‾‾\
//    _____/          \_____
//
//    ^ up   ^ down  ^ released
//         ^ pressed   ^ up

enum {
	// down -> PRESSED || HELD
	// up   -> NOTHING || RELEASED
	INSTATE_UP        = 0x00,
	INSTATE_PRESSED   = 0x01,
	INSTATE_DOWN      = 0x02,
	INSTATE_RELEASED  = 0x03,
	INSTATE_TOTAL
};

// INPUT_KEY
//   These are the keys you get to use :)

enum {
	INPUT_KEY__START,
	// numerics
	INPUT_KEY_0,
	INPUT_KEY_1,
	INPUT_KEY_2,
	INPUT_KEY_3,
	INPUT_KEY_4,
	INPUT_KEY_5,
	INPUT_KEY_6,
	INPUT_KEY_7,
	INPUT_KEY_8,
	INPUT_KEY_9,

	// a-z keys in qwerty layout
	INPUT_KEY_Q,
	INPUT_KEY_W,
	INPUT_KEY_E,
	INPUT_KEY_R,
	INPUT_KEY_T,
	INPUT_KEY_Y,
	INPUT_KEY_U,
	INPUT_KEY_I,
	INPUT_KEY_O,
	INPUT_KEY_P,
	INPUT_KEY_A,
	INPUT_KEY_S,
	INPUT_KEY_D,
	INPUT_KEY_F,
	INPUT_KEY_G,
	INPUT_KEY_H,
	INPUT_KEY_J,
	INPUT_KEY_K,
	INPUT_KEY_L,
	INPUT_KEY_Z,
	INPUT_KEY_X,
	INPUT_KEY_C,
	INPUT_KEY_V,
	INPUT_KEY_B,
	INPUT_KEY_N,
	INPUT_KEY_M,

	INPUT_KEY_UARROW,
	INPUT_KEY_RARROW,
	INPUT_KEY_DARROW,
	INPUT_KEY_LARROW,

	// modifier keys
	INPUT_KEY_SHIFT,
	INPUT_KEY_TAB,

	// extras
	INPUT_KEY_ESC,
	INPUT_KEY_SPACE,
	INPUT_KEY_HOME,
	INPUT_KEY_END,
	INPUT_KEY_CTRL,
	INPUT_KEY__END,

	// mouse "keys"
	INPUT_MOUSE__START,
	INPUT_MOUSE_LEFT,
	INPUT_MOUSE_RIGHT,
	INPUT_MOUSE_CENTER,
	INPUT_MOUSE__END,

	// controller "keys"
	INPUT_CTRLLR__START,
	INPUT_CTRLLR_A,
	INPUT_CTRLLR_B,
	INPUT_CTRLLR_X,
	INPUT_CTRLLR_Y,
	INPUT_CTRLLR_BACK,
	INPUT_CTRLLR_START,
	INPUT_CTRLLR_LSTICK,
	INPUT_CTRLLR_RSTICK,
	INPUT_CTRLLR_LSHOULDER,
	INPUT_CTRLLR_RSHOULDER,
	INPUT_CTRLLR_DPAD_U,
	INPUT_CTRLLR_DPAD_D,
	INPUT_CTRLLR_DPAD_L,
	INPUT_CTRLLR_DPAD_R,
	INPUT_CTRLLR__END,

	INPUT_KEY_TOTAL
};

struct io_t {
	s32 sig_quit;
	s32 __placeholder__;

	s8 keys[INPUT_KEY_TOTAL];

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

