// ////////////////////////////////////////////////////////////////////////////
// BRIAN CHRZANOWSKI
// 2020-12-04 22:46:05
//
// My own terminal
//
// TODO (Brian)
// ////////////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////
// ////////////////////////////////////////////////////////////////////////////

#define SDL_MAIN_HANDLED
#include <SDL.h>

// NOTE (Brian) we have the undefs so any headers we include won't include the implementation twice
#define COMMON_IMPLEMENTATION
#include "common.h"
#undef COMMON_IMPLEMENTATION

// include all of the libraries first
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#undef STB_TRUETYPE_IMPLEMENTATION

// NOTE (Brian) define some constants that should probably go in a config file at some point
#define ROWS (70)
#define COLS (120)

#define SCREEN_WIDTH  (1280)
#define SCREEN_HEIGHT (960)

#define FONT_HEIGHT   (SCREEN_HEIGHT / ROWS)
// #define FONT_WIDTH    (SCREEN_WIDTH / COLS)

#include "font.h"
#include "io.h"

struct color_t {
	u8 r, g, b, a;
};

struct state_t {
	s32 run;
	s32 rows, cols;
	struct color_t fg, bg;
	struct font_t font;
	struct io_t io;
	char display[ROWS][COLS];
	s32 curr_x, curr_y;
};

// STARTUP / SHUTDOWN FUNCTIONS
/* Init : Initializes the Game State */
s32 Init();

// Close : closes the application
s32 Close(struct state_t *state);

// Run : runs the app
s32 Run(struct state_t *state);

// Update : the game update function
void Update(struct state_t *state);

// UpdateTermDimensions : updates the terminal's dimensions
void UpdateTermDimensions(struct state_t *state);

// RENDER FUNCTIONS
// Render : the game render function
void Render(struct state_t *state);
// RenderString : render a string to the given (x,y) position with height h
void RenderString(struct state_t *state, s32 assetidx, char *s, s32 x, s32 y, s32 h);
// RenderChar : render a single character
void RenderChar(struct fontchar_t *fontchar, SDL_Rect r, struct color_t color);

// Delay : conditional delay, as needed
void Delay(struct state_t *state);

// UtilMakeColor : returns a color
struct color_t UtilMakeColor(u8 r, u8 g, u8 b, u8 a);

// NOTE (Brian) globals are fine if they aren't in a library
SDL_Window *gWindow;
SDL_Renderer *gRenderer;

int main(int argc, char **argv)
{
	struct state_t state;

	Init(&state);
	Run(&state);
	Close(&state);

	return 0;
}

// Run : runs the app
s32 Run(struct state_t *state)
{
	assert(state);

	char *s = "Hello World, how are you today!?";

	{
		s32 i, j, k;

		k = 0;
#if 0
		for (i = 0; i < strlen(s) && i < 80 * 24; i++) {
			((char *)state->display)[i] = s[i];
		}
#else
		memset(state->display, ' ', sizeof state->display);
		for (i = 0; i < ARRSIZE(state->display); i++) {
			for (j = 0; j < ARRSIZE(state->display[i]); j++) {
				state->display[i][j] = s[k++ % strlen(s)];
			}
		}

		for (i = 0; i < sizeof(state->display); i++) {
			printf("%d - %c\n", i, ((char *)(void *)state->display)[i]);
		}
#endif
	}

	while (state && state->run && !state->io.sig_quit) {
		InputRead(&state->io);
		Update(state);
		Render(state);
		Delay(state);
	}

	return 0;
}

// Update : the game update function
void Update(struct state_t *state)
{
	assert(state);

	UpdateTermDimensions(state);
}

// UpdateTermDimensions : updates the terminal's dimensions
void UpdateTermDimensions(struct state_t *state)
{
	assert(state);

#if 0
	state->rows = state->io.win_h / FONT_HEIGHT;
	state->cols = state->io.win_w / FONT_WIDTH;

	printf("WinW : %d, WinH : %d\n", state->io.win_w, state->io.win_h);
	printf("Rows : %d, Cols : %d\n", state->rows, state->cols);
#else
	state->rows = ROWS;
	state->cols = COLS;
#endif
}

// Render : the game render function
void Render(struct state_t *state)
{
	struct font_t *font;
	struct fontchar_t *fontchar;
	s32 i, j, c;
	s32 x, y;
	SDL_Rect r;

	font = &state->font;

	x = 0;
	y = font->ascent;

	// clear the screen
	SDL_SetRenderDrawColor(gRenderer, state->bg.r, state->bg.g, state->bg.b, state->bg.a);
	SDL_RenderClear(gRenderer);

	for (i = 0; i < state->rows; i++) {
		for (j = 0; j < state->cols; j++) {

			c = state->display[i][j];
			fontchar = &state->font.fonttab[(int)c];

			r.x = x + fontchar->b_x;
			r.y = y + fontchar->b_y;
			r.w = fontchar->f_x;
			r.h = fontchar->f_y;

			printf("%c - (%d,%d) (%d,%d)\n", c, r.x, r.y, r.w, r.h);

			RenderChar(state->font.fonttab + c, r, state->fg);

			x += fontchar->advance;
		}

		x = 0;
		y += font->vertadvance;
	}

	// present the screen
	SDL_RenderPresent(gRenderer);
}

// RenderChar : render a single character (testing)
void RenderChar(struct fontchar_t *fontchar, SDL_Rect r, struct color_t color)
{
	// NOTE (Brian) font is the asset index
	// TODO (Brian) we probably want to pass in color information too

	assert(fontchar);

	// NOTE (Brian) we assume ascii is the thing, update to use a _real_ unicode thing later
	if (isspace(fontchar->codepoint)) {
		return;
	}

	if (!fontchar->is_valid) {
		return;
	}

	// SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
	SDL_SetTextureColorMod(fontchar->texture, color.r, color.g, color.b);

	// copy the font to the destination rectangle
	SDL_RenderCopy(gRenderer, fontchar->texture, NULL, &r);
}

// Delay : conditional delay, as needed
void Delay(struct state_t *state)
{
	SDL_Delay(3); // NOTE (Brian) TEMPORARY!!
}

// Init : Initializes the Game State
s32 Init(struct state_t *state)
{
	s32 rc;

	assert(state);

	memset(state, 0, sizeof(*state));

	// setup SDL before we do anything
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		ERR("Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	u32 flags;

	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, flags, &gWindow, &gRenderer);
	if (rc < 0) { // error
		ERR("Couldn't Create Window or Renderer: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetWindowTitle(gWindow, "Brian's Terminal");

	rc = SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	if (rc < 0) {
		ERR("Couldn't set render blend mode: %s\n", SDL_GetError());
		return -1;
	}

	// TEMPORARY (Brian) 
	FontLoad(&state->font, "./assets/fonts/LiberationMono-Regular.ttf", FONT_HEIGHT);
	state->run = 1;

	state->fg = UtilMakeColor(0xff, 0xff, 0xff, 0xff);
	state->bg = UtilMakeColor(0x00, 0x00, 0x00, 0xff);

	return 0;
}

// Close : closes the application
s32 Close(struct state_t *state)
{
	assert(state);

	if (gRenderer)
		SDL_DestroyRenderer(gRenderer);

	if (gWindow)
		SDL_DestroyWindow(gWindow);

	SDL_Quit();

	return 0;
}

// UtilMakeColor : returns a color
struct color_t UtilMakeColor(u8 r, u8 g, u8 b, u8 a)
{
	struct color_t c = { r, g, b, a };
	return c;
}

