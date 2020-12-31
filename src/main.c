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

#include <math.h>

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
	f32 px, py;   // center
	f32 vx, vy;
	f32 rotation; // in deg
	s32 is_firing;
	s32 is_flying;
};

struct asteroid_t {
	f32 px, py;
	f32 vx, vy;
	f32 rotation;
	f32 rotation_vel;
};

struct state_t {
	s32 run;
	s32 rows, cols;

	u32 ticks;

	struct player_t player;

	struct asteroid_t *asteroids;
	size_t asteroids_len, asteroids_cap;

	struct asset_container_t asset_container;

	struct io_t io;
};

// STARTUP / SHUTDOWN FUNCTIONS
// Init : Initializes the Game State
s32 Init();

// InitAssets : loads assets
s32 InitAssets(struct state_t *state);

// InitPlayer : initializes the player
s32 InitPlayer(struct state_t *state);

// InitAsteroids : initializes asteroids
s32 InitAsteroids(struct state_t *state);

// Close : closes the application
s32 Close(struct state_t *state);

// Run : runs the app
s32 Run(struct state_t *state);

// Update : the game update function
void Update(struct state_t *state);

// Update : updates the player
void UpdatePlayer(struct state_t *state);

// UpdateAsteroids : updates all of the asteroids
void UpdateAsteroids(struct state_t *state);

// RENDER FUNCTIONS
// Render : the game render function
void Render(struct state_t *state);

// RenderPlayer : renders the player to the screen
void RenderPlayer(struct state_t *state);

// RenderAsteroids : renders all of the asteroids
void RenderAsteroids(struct state_t *state);

// Delay : conditional delay, as needed
void Delay(struct state_t *state);

// UtilMakeColor : returns a color
struct color_t UtilMakeColor(u8 r, u8 g, u8 b, u8 a);

// RandInt : returns a random int in [min, max]
s32 RandInt(s32 min, s32 max);

// RandFloat : returns a random float (mostly) in [min, max]
f32 RandFloat(f32 min, f32 max);

// WrapCoord : wraps the coordinate to the min and max
void WrapCoord(f32 *coord, f32 min, f32 max);

// NOTE (Brian) globals are fine if they aren't in a library
SDL_Window *gWindow;
SDL_Renderer *gRenderer;

int main(int argc, char **argv)
{
	struct state_t state;

	srand(time(NULL));

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

		state->ticks++;
	}

	return 0;
}

// Update : the game update function
void Update(struct state_t *state)
{
	assert(state);

	UpdatePlayer(state);
	UpdateAsteroids(state);
}

void UpdatePlayer(struct state_t *state)
{
	struct player_t *player;
	struct io_t *io;

	player = &state->player;
	io = &state->io;

	player->is_flying = 0;
	player->is_firing = io->key_a;

#define ACCELERATION (M_PI / 2 / 20)

	if (io->key_e) {
		player->rotation += ACCELERATION;
	}

	if (io->key_w) {
		player->rotation -= ACCELERATION;
	}

	if (io->key_n) {
		player->vx -= cos(player->rotation) * ACCELERATION;
		player->vy -= sin(player->rotation) * ACCELERATION;
		player->is_flying = 1;
	}

	player->px += player->vx;
	player->py += player->vy;

	WrapCoord(&player->px, 0, GAMERES_WIDTH);
	WrapCoord(&player->py, 0, GAMERES_HEIGHT);
}

// UpdateAsteroids : updates all of the asteroids
void UpdateAsteroids(struct state_t *state)
{
	s32 i;
	struct asteroid_t *asteroid;

	for (i = 0; i < state->asteroids_len; i++) {
		asteroid = state->asteroids + i;
		asteroid->px += asteroid->vx;
		asteroid->py += asteroid->vy;
		asteroid->rotation += asteroid->rotation_vel;

		WrapCoord(&asteroid->px, 0, GAMERES_WIDTH);
		WrapCoord(&asteroid->py, 0, GAMERES_HEIGHT);
	}
}

// Render : the game render function
void Render(struct state_t *state)
{
	// clear the screen
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
	SDL_RenderClear(gRenderer);

	RenderAsteroids(state);

	RenderPlayer(state);

	// present the screen
	SDL_RenderPresent(gRenderer);
}

// RenderPlayer : renders the player to the screen
void RenderPlayer(struct state_t *state)
{
	struct player_t *player;
	struct asset_t *a_ship, *a_shipgun, *a_shipthruster;
	SDL_Rect dst;
	f32 degrotation;

	assert(state);

	player = &state->player;

	// load up all of the assets we'll need
	a_ship         = AssetFetchByName(&state->asset_container, "ship");
	a_shipgun      = AssetFetchByName(&state->asset_container, "shipguns");
	a_shipthruster = AssetFetchByName(&state->asset_container, "shipthruster");

	assert(a_ship);
	assert(a_shipgun);
	assert(a_shipthruster);

	// gather the destination information FIRST
	dst.w = a_ship->w;
	dst.h = a_ship->h;
	dst.x = player->px - dst.w / 2;
	dst.y = player->py - dst.h / 2;

	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0xff, 0xff);
	SDL_RenderDrawRect(gRenderer, &dst);

	degrotation = ((player->rotation - M_PI / 2) * 180 / M_PI);

	// then, draw all of the pieces
	SDL_RenderCopyEx(gRenderer, a_ship->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);

	if (player->is_firing) {
		SDL_RenderCopyEx(gRenderer, a_shipgun->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);
	}

	if (player->is_flying) {
		SDL_RenderCopyEx(gRenderer, a_shipthruster->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);
	}
}

// RenderAsteroids : renders all of the asteroids
void RenderAsteroids(struct state_t *state)
{
	s32 i;
	struct asteroid_t *asteroid;
	struct asset_t *a_asteroid;
	SDL_Rect dst;
	f32 degrotation;

	assert(state);

	// load up all of the assets we'll need
	a_asteroid = AssetFetchByName(&state->asset_container, "asteroid");

	assert(a_asteroid);

	for (i = 0; i < state->asteroids_len; i++) {

		asteroid = state->asteroids + i;

		// gather the destination information FIRST
		dst.w = a_asteroid->w;
		dst.h = a_asteroid->h;
		dst.x = asteroid->px - dst.w / 2;
		dst.y = asteroid->py - dst.h / 2;

		SDL_SetRenderDrawColor(gRenderer, 0xff, 0, 0, 0xff);
		SDL_RenderDrawRect(gRenderer, &dst);

		degrotation = ((asteroid->rotation - M_PI / 2) * 180 / M_PI);

		// then, draw all of the pieces
		SDL_RenderCopyEx(gRenderer, a_asteroid->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);
	}
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

	InitPlayer(state);
	InitAsteroids(state);

	state->run = 1;

	return 0;
}

// InitAssets : loads assets
s32 InitAssets(struct state_t *state)
{
	assert(state);

	// load all of the ship assets
	AssetLoad(&state->asset_container, "assets/sprites/ship.png");
	AssetLoad(&state->asset_container, "assets/sprites/shipguns.png");
	AssetLoad(&state->asset_container, "assets/sprites/shipthruster.png");

	// load the ship projectile
	// AssetLoad(&state->asset_container, "assets/sprites/bullet.png");

	// load the asteroids
	AssetLoad(&state->asset_container, "assets/sprites/asteroid.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_1.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_2.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_3.png");
	// AssetLoad(&state->asset_container, "assets/sprites/asteroid_4.png");

	return 0;
}

// InitPlayer : initializes the player
s32 InitPlayer(struct state_t *state)
{
	struct player_t *player;

	player = &state->player;

	player->px = GAMERES_WIDTH / 2;
	player->py = GAMERES_HEIGHT / 2;

	player->vx = 0.0;
	player->vy = 0.0;

	player->rotation = M_PI / 2;

	return 0;
}

// InitAsteroids : initializes asteroids
s32 InitAsteroids(struct state_t *state)
{
	struct asteroid_t *asteroid;
	s32 i, n;

	for (i = 0, n = RandInt(20, 30); i < n; i++) {
		C_RESIZE(&state->asteroids);

		asteroid = state->asteroids + i;

		asteroid->px = RandInt(0, GAMERES_WIDTH);
		asteroid->py = RandInt(0, GAMERES_HEIGHT);

		asteroid->vx = RandFloat(-ACCELERATION, ACCELERATION) * RandFloat(1.0, 10.0);
		asteroid->vy = RandFloat(-ACCELERATION, ACCELERATION) * RandFloat(1.0, 10.0);
		asteroid->rotation = RandFloat(-ACCELERATION, ACCELERATION);
		asteroid->rotation_vel = ACCELERATION * RandFloat(-1.0, 1.0);

		state->asteroids_len++;
	}

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

// RandInt : returns a random int in [min, max]
s32 RandInt(s32 min, s32 max)
{
	// TODO (brian) replace me with the better prng thing
	return (rand() % (max - min + 1)) + min;
}

// RandFloat : returns a random float (mostly) in [min, max]
f32 RandFloat(f32 min, f32 max)
{
	f32 scale;
	scale = rand() / (float)RAND_MAX;
	return scale * (max - min) + min;
}

// WrapCoord : wraps the coordinate to the min and max
void WrapCoord(f32 *coord, f32 min, f32 max)
{
	f32 overhang;

	overhang = max * 0.02;

	if (*coord > max + overhang) {
		*coord = min - overhang;
	}

	if (*coord < min - overhang) {
		*coord = max + overhang;
	}

}

