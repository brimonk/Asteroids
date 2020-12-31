// ////////////////////////////////////////////////////////////////////////////
// BRIAN CHRZANOWSKI
// 2020-12-04 22:46:05
//
// My version of Asteroids
//
// TODO (Brian)
// ////////////////////////////////////////////////////////////////////////////

#define SDL_MAIN_HANDLED
#include <SDL.h>

// NOTE (Brian) we have the undefs so any headers we include won't include the implementation twice
#define COMMON_IMPLEMENTATION
#include "common.h"
#undef COMMON_IMPLEMENTATION

// include all of the libraries first
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

// NOTE (Brian) define some constants that should probably go in a config file at some point

#define ROWS (48)
#define COLS (100)

#define SCREEN_WIDTH   (1280)
#define SCREEN_HEIGHT  (720)

#define GAMERES_WIDTH  (640)
#define GAMERES_HEIGHT (480)

#define WINDOW_NAME ("Asteroids")

#include "io.h"
#include "asset.h"

struct color_t {
	u8 r, g, b, a;
};

struct player_t {
	f32 x, y; // center
	f32 rotation; // in deg
};

struct state_t {
	s32 run;
	s32 rows, cols;

	u32 ticks;

	struct player_t player;
	struct asset_container_t asset_container;

	struct io_t io;
};

// STARTUP / SHUTDOWN FUNCTIONS
// Init : Initializes the Game State
s32 Init();

// InitAssets : loads assets
s32 InitAssets(struct state_t *state);

// Close : closes the application
s32 Close(struct state_t *state);

// Run : runs the app
s32 Run(struct state_t *state);

// Update : the game update function
void Update(struct state_t *state);

// RENDER FUNCTIONS
// Render : the game render function
void Render(struct state_t *state);

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
}

// Render : the game render function
void Render(struct state_t *state)
{
	struct color_t cfg, cbg;
	s32 i, j, c;
	s32 x, y;
	SDL_Rect r;

	x = 2;

	// clear the screen
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
	SDL_RenderClear(gRenderer);

	// present the screen
	SDL_RenderPresent(gRenderer);
}

// Delay : conditional delay, as needed
void Delay(struct state_t *state)
{
	SDL_Delay(16); // NOTE (Brian) TEMPORARY!!
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

	flags = SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE;

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, flags, &gWindow, &gRenderer);
	if (rc < 0) { // error
		ERR("Couldn't Create Window or Renderer: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetWindowTitle(gWindow, WINDOW_NAME);

	rc = SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	if (rc < 0) {
		ERR("Couldn't set render blend mode: %s\n", SDL_GetError());
		return -1;
	}

	rc = SDL_RenderSetLogicalSize(gRenderer, GAMERES_WIDTH, GAMERES_HEIGHT);
	if (rc < 0) {
		ERR("Couldn't set renderer logical size: %s\n", SDL_GetError());
		return -1;
	}

	InitAssets(state);

	state->run = 1;

	return 0;
}

// InitAssets : loads assets
s32 InitAssets(struct state_t *state)
{
	assert(state);

	// load all of the ship assets
	AssetLoad(&state->asset_container, "assets/sprites/ship.png");
	AssetLoad(&state->asset_container, "assets/sprites/ship_guns.png");
	AssetLoad(&state->asset_container, "assets/sprites/ship_thruster.png");

	// load the ship projectile
	// AssetLoad(&state->asset_container, "assets/sprites/bullet.png");

	// load the asteroids (planning on having 5)
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_0.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_1.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_2.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_3.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_4.png");

	return 0;
}

// Close : closes the application
s32 Close(struct state_t *state)
{
	assert(state);

	AssetsFree(&state->asset_container);

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

