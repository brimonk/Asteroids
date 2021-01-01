/*
 * Brian Chrzanowski
 * 2020-12-31 15:30:57
 *
 * Asteroids
 *
 * NOTE
 *
 * There's currently an open learning question, which is, how should the collisions
 * actually be detected. I've tried several things (point in rect check, rect overlap check, ray
 * line intersection check) none of which actually worked.
 *
 * For now, the thing that _seems_ to work well enough is computing the distance between the center
 * of the bullet and the asteroid, and if it's less than a constant (TBD what this should be) it
 * seems to be "good enough".
 */

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

#define ACCELERATION (M_PI / 2 / 20)

#define WINDOW_NAME ("Asteroids")

#include "io.h"
#include "asset.h"

struct color_t {
	u8 r, g, b, a;
};

typedef struct vec2f {
	f32 x, y;
} vec2f;

typedef vec2f point;

typedef struct line {
	point p1, p2;
} line;

// movement_t : describes position, velocity, and acceleration
struct movement_t {
	// position, velocity, and acceleration in (x, y)
	f32 px, py;
	f32 vx, vy;
	f32 ax, ay;

	// position, velocity, and acceleration for rotation (in radians)
	// NOTE the naming is like, bad for these... :(
	f32 pr;
	f32 pv;
	f32 pa;
};

struct entity_t {
	s32 type;
};

struct player_t {
	struct entity_t entity;
	struct movement_t movement;
	s32 is_flying;
	s32 has_fired;
};

struct asteroid_t {
	struct entity_t entity;
	struct movement_t movement;
	s32 is_used;
};

struct bullet_t {
	struct entity_t entity;
	struct movement_t movement;
	s32 is_used;
};

struct state_t {
	s32 run;
	s32 rows, cols;

	u32 ticks;

	struct player_t player;

	struct asteroid_t *asteroids;
	size_t asteroids_len, asteroids_cap;

	struct bullet_t bullets[4096];
	s32 bullet_next;

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

// UpdateBullets : updates all of the bullets
void UpdateBullets(struct state_t *state);

// UpdateMovement : updates the individual movement instance
void UpdateMovement(struct movement_t *movement);

// CheckBulletAsteroidCollision : what it sounds like
s32 CheckBulletAsteroidCollision(struct state_t *state, s32 aidx, s32 bidx);

// RENDER FUNCTIONS
// Render : the game render function
void Render(struct state_t *state);

// RenderPlayer : renders the player to the screen
void RenderPlayer(struct state_t *state);

// RenderAsteroids : renders all of the asteroids
void RenderAsteroids(struct state_t *state);

// RenderBullets : renders all of the bullets
void RenderBullets(struct state_t *state);

// CreateBullet : creates a bullet at (px, py) with velocity (vx, vy)
void CreateBullet(struct state_t *state, f32 px, f32 py, f32 vx, f32 vy);

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

// IsOOB : is out of bounds?
s32 IsOOB(f32 cx, f32 cy, f32 w, f32 h);

// Vec2fCross : compute the cross product of two vec2fs
f32 Vec2fCross(vec2f a, vec2f b);

// Vec2fSub : compute the subtraction of two vec2fs
vec2f Vec2fSub(vec2f a, vec2f b);

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
	UpdateBullets(state);
	UpdateAsteroids(state);
}

// UpdatePlayer : updates the player
void UpdatePlayer(struct state_t *state)
{
	struct player_t *player;
	struct movement_t *movement;
	struct io_t *io;

	io = &state->io;
	player = &state->player;
	movement = &player->movement;

	player->is_flying = 0;

	// I know it says acceleration here, but just go with it for now, okay

	// NOTE (brian) we should (maybe) make UpdateMovement work with this too...

	player->movement.pv = 0.0;

	if (io->key_e) {
		player->movement.pv += ACCELERATION;
	}

	if (io->key_w) {
		player->movement.pv -= ACCELERATION;
	}

	player->movement.pr += player->movement.pv;

	if (io->key_n) {
		// the clocwiseness of sdl is weird, but it checks out
		player->movement.vx -= cos(player->movement.pr) * ACCELERATION;
		player->movement.vy -= sin(player->movement.pr) * ACCELERATION;
		player->is_flying = 1;
	}

	player->movement.px += player->movement.vx;
	player->movement.py += player->movement.vy;

	// create the bullet after we compute motion
	if (io->key_a) {
		f32 bvx, bvy;
		if (!player->has_fired) {
			bvx = -(cos(player->movement.pr) * ACCELERATION * 60);
			bvy = -(sin(player->movement.pr) * ACCELERATION * 60);
			CreateBullet(state, player->movement.px, player->movement.py, bvx, bvy);
			player->has_fired = 1;
		}
	} else {
		player->has_fired = 0;
	}

	WrapCoord(&player->movement.px, 0, GAMERES_WIDTH);
	WrapCoord(&player->movement.py, 0, GAMERES_HEIGHT);
}

// UpdateAsteroids : updates all of the asteroids
void UpdateAsteroids(struct state_t *state)
{
	s32 i;
	struct asteroid_t *asteroid;

	for (i = 0; i < state->asteroids_len; i++) {
		asteroid = state->asteroids + i;

		if (!asteroid->is_used)
			continue;

		UpdateMovement(&asteroid->movement);

		WrapCoord(&asteroid->movement.px, 0, GAMERES_WIDTH);
		WrapCoord(&asteroid->movement.py, 0, GAMERES_HEIGHT);
	}
}

// CreateBullet : creates a bullet at (px, py) with velocity (vx, vy)
void CreateBullet(struct state_t *state, f32 px, f32 py, f32 vx, f32 vy)
{
	struct bullet_t *bullet;

	bullet = &state->bullets[state->bullet_next++ % ARRSIZE(state->bullets)];
	state->bullet_next %= ARRSIZE(state->bullets);

	bullet->movement.px = px;
	bullet->movement.py = py;
	bullet->movement.vx = vx;
	bullet->movement.vy = vy;

	bullet->movement.pr = tan((bullet->movement.px + bullet->movement.vx) /
				(bullet->movement.py + bullet->movement.vy));

	bullet->is_used = 1;
}

// Point : makes a point
point Point(f32 x, f32 y)
{
	point a = { x, y };
	return a;
}

// Distance : computes the distance between p1 and p2
f32 Distance(point p1, point p2)
{
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

// UpdateBullets : updates all of the bullets
void UpdateBullets(struct state_t *state)
{
	struct bullet_t *bullet;
	struct asteroid_t *asteroid;
	s32 i, j;

	// NOTE (brian) we have to check if the 'proposed' movement vector is going
	// to collide with the _each_ asteroid.
	//
	// This is pretty dang expensive, so I'm wondering if, in the future, if
	// this is too slow to do each frame, we sort the asteroids every frame
	// instead, like, per quad or something.
	//
	// Just a thought.

	// update position

	for (i = 0; i < ARRSIZE(state->bullets); i++) {
		bullet = state->bullets + i;

		if (!bullet->is_used)
			continue;

		if (IsOOB(bullet->movement.px, bullet->movement.py, GAMERES_WIDTH, GAMERES_HEIGHT)) {
			bullet->is_used = 0;
			continue;
		}

		UpdateMovement(&state->bullets[i].movement);


		// collision checks (this can potentially skip over some asteroids)
		for (j = 0; j < state->asteroids_len; j++) {
			point bp, ap;

			asteroid = state->asteroids + j;
			if (!asteroid->is_used)
				continue;

			ap = Point(asteroid->movement.px, asteroid->movement.py);
			bp = Point(bullet->movement.px, bullet->movement.py);

			// TODO (brian) this would be better if we attempted to circumscribe the asteroid (and
			// bullet?) with a circle, and check if the distance from the bullet to the asteroid is
			// less than the radius of the circle. 8 is just a filler because the asteroid sprite
			// size is 16, so sqrt(8 * 8)

			if (Distance(ap, bp) < 8) {
				bullet->is_used = 0;
				asteroid->is_used = 0;
				break;
			}
		}
	}
}

// UpdateMovement : updates the individual movement instance
void UpdateMovement(struct movement_t *movement)
{
	movement->vx += movement->ax;
	movement->vy += movement->ay;
	movement->pv += movement->pa;

	movement->px += movement->vx;
	movement->py += movement->vy;
	movement->pr += movement->pv;
}

// Render : the game render function
void Render(struct state_t *state)
{
	// clear the screen
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
	SDL_RenderClear(gRenderer);

	RenderAsteroids(state);
	RenderPlayer(state);
	RenderBullets(state);

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
	dst.x = player->movement.px - dst.w / 2;
	dst.y = player->movement.py - dst.h / 2;

	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0xff, 0xff);
	SDL_RenderDrawRect(gRenderer, &dst);

	degrotation = ((player->movement.pr - M_PI / 2) * 180 / M_PI);

	// then, draw all of the pieces
	SDL_RenderCopyEx(gRenderer, a_ship->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);

	if (player->has_fired) {
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

		if (!asteroid->is_used)
			continue;

		// gather the destination information FIRST
		dst.w = a_asteroid->w;
		dst.h = a_asteroid->h;
		dst.x = asteroid->movement.px - dst.w / 2;
		dst.y = asteroid->movement.py - dst.h / 2;

		SDL_SetRenderDrawColor(gRenderer, 0xff, 0, 0, 0xff);
		SDL_RenderDrawRect(gRenderer, &dst);

		degrotation = ((asteroid->movement.pr - M_PI / 2) * 180 / M_PI);

		// then, draw all of the pieces
		SDL_RenderCopyEx(gRenderer, a_asteroid->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);
	}
}

// RenderBullets : renders all of the bullets
void RenderBullets(struct state_t *state)
{
	s32 i;
	struct bullet_t *bullet;
	struct asset_t *a_bullet;
	SDL_Rect dst;
	f32 degrotation;

	assert(state);

	// load up all of the assets we'll need
	a_bullet = AssetFetchByName(&state->asset_container, "bullet");

	assert(a_bullet);

	for (i = 0; i < ARRSIZE(state->bullets); i++) {

		bullet = state->bullets + i;

		if (!bullet->is_used)
			continue;

		// gather the destination information FIRST
		dst.w = a_bullet->w;
		dst.h = a_bullet->h;
		dst.x = bullet->movement.px - dst.w / 2;
		dst.y = bullet->movement.py - dst.h / 2;

		SDL_SetRenderDrawColor(gRenderer, 0, 0xff, 0, 0xff);
		SDL_RenderDrawRect(gRenderer, &dst);

		degrotation = ((bullet->movement.pr - M_PI / 2) * 180 / M_PI);

		// then, draw all of the pieces
		SDL_RenderCopyEx(gRenderer, a_bullet->texture, NULL, &dst, degrotation, NULL, SDL_FLIP_NONE);
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
	AssetLoad(&state->asset_container, "assets/sprites/bullet.png");

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

	player->movement.px = GAMERES_WIDTH / 2;
	player->movement.py = GAMERES_HEIGHT / 2;

	player->movement.vx = 0.0;
	player->movement.vy = 0.0;

	player->movement.ax = 0.0;
	player->movement.ay = 0.0;

	player->movement.pr = M_PI / 2; // face upwards
	player->movement.pv = 0.0;
	player->movement.pa = 0.0;

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

		asteroid->is_used = 1;

		asteroid->movement.px = RandInt(0, GAMERES_WIDTH);
		asteroid->movement.py = RandInt(0, GAMERES_HEIGHT);

		asteroid->movement.vx = RandFloat(-ACCELERATION, ACCELERATION) * RandFloat(1.0, 10.0);
		asteroid->movement.vy = RandFloat(-ACCELERATION, ACCELERATION) * RandFloat(1.0, 10.0);
		asteroid->movement.pr = RandFloat(-ACCELERATION, ACCELERATION);
		asteroid->movement.pv = ACCELERATION * RandFloat(-1.0, 1.0);

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

// IsOOB : is out of bounds?
s32 IsOOB(f32 cx, f32 cy, f32 w, f32 h)
{
	return (cx < 0 || cx > w) || (cy < 0 || cy > h);
}

