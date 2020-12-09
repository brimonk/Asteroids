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

#define ROWS (24)
#define COLS (80)
#define FONT_HEIGHT   (26)

#define SCREEN_WIDTH  (1280)
#define SCREEN_HEIGHT (720)

// #define FONT_HEIGHT   (SCREEN_HEIGHT / ROWS)
// #define FONT_WIDTH    (SCREEN_WIDTH / COLS)

#include "font.h"
#include "io.h"

enum {
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_BROWN,
	COLOR_YELLOW,
	COLOR_LGRAY,
	COLOR_DGRAY,
	COLOR_LBLUE,
	COLOR_LGREEN,
	COLOR_LCYAN,
	COLOR_LRED,
	COLOR_LMAGENTA,
	COLOR_WHITE,
};

struct color_t {
	u8 r, g, b;
};

struct termchar_t {
	u32 codepoint;
	s32 fg;
	s32 bg;
	u32 flags;
};

struct state_t {
	s32 run;
	s32 rows, cols;

	struct font_t font;
	struct io_t io;

	s32 fg, bg;
	struct color_t colors[16];

	char display[ROWS][COLS];
	s32 curr_x, curr_y;
};

// STARTUP / SHUTDOWN FUNCTIONS
// Init : Initializes the Game State
s32 Init();
// InitColors : initialize colors
void InitColors(struct state_t *state);

// Close : closes the application
s32 Close(struct state_t *state);

// Run : runs the app
s32 Run(struct state_t *state);

// Update : the game update function
void Update(struct state_t *state);

// UpdateClearScreen : clears the screen
void UpdateClearScreen(struct state_t *state);

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
struct color_t UtilMakeColor(u8 r, u8 g, u8 b);

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

// UpdateClearScreen : clears the screen
void UpdateClearScreen(struct state_t *state)
{
	memset(state->display, ' ', sizeof state->display);
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
	struct color_t cfg, cbg;
	s32 i, j, c;
	s32 x, y;
	SDL_Rect r;

	font = &state->font;

	x = 2;
	y = font->ascent;

	state->fg = COLOR_WHITE;
	state->bg = COLOR_BLACK;

	cfg = state->colors[state->fg];
	cbg = state->colors[state->bg];

	// clear the screen
	SDL_SetRenderDrawColor(gRenderer, cbg.r, cbg.g, cbg.b, 0xff);
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

			RenderChar(state->font.fonttab + c, r, cfg);

			x += fontchar->advance;
		}

		x = 2;
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
	FontLoad(&state->font, "./assets/fonts/Px437_IBM_EGA_8x14.ttf", FONT_HEIGHT);
	state->run = 1;

	InitColors(state);

	return 0;
}

// InitColors : initialize colors
void InitColors(struct state_t *state)
{
	state->colors[COLOR_BLACK]    = UtilMakeColor(38, 23, 10);
	state->colors[COLOR_BLUE]     = UtilMakeColor(15, 82, 186);
	state->colors[COLOR_GREEN]    = UtilMakeColor(120, 134, 23);
	state->colors[COLOR_CYAN]     = UtilMakeColor(86, 184, 114);
	state->colors[COLOR_RED]      = UtilMakeColor(132, 0, 0);
	state->colors[COLOR_MAGENTA]  = UtilMakeColor(124, 26, 96);
	state->colors[COLOR_BROWN]    = UtilMakeColor(104, 75, 58);
	state->colors[COLOR_YELLOW]   = UtilMakeColor(255, 195, 34);
	state->colors[COLOR_LGRAY]    = UtilMakeColor(154, 132, 109);
	state->colors[COLOR_DGRAY]    = UtilMakeColor(65, 53, 43);
	state->colors[COLOR_LBLUE]    = UtilMakeColor(0, 138, 255);
	state->colors[COLOR_LGREEN]   = UtilMakeColor(196, 219, 38);
	state->colors[COLOR_LCYAN]    = UtilMakeColor(72, 255, 184);
	state->colors[COLOR_LRED]     = UtilMakeColor(192, 61, 36);
	state->colors[COLOR_LMAGENTA] = UtilMakeColor(255, 66, 130);
	state->colors[COLOR_WHITE]    = UtilMakeColor(252, 250, 208);

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
struct color_t UtilMakeColor(u8 r, u8 g, u8 b)
{
	struct color_t c = { r, g, b };
	return c;
}

