/*
 * Brian Chrzanowski
 * 2020-12-31 15:30:57
 *
 * Asteroids
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

// IsInside : returns true if the point lays inside the polygon of n sides
s32 IsInside(vec2f *polygon, s32 n, vec2f point);

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

void VerboseLogging(struct state_t *state)
{
	struct player_t *player;
	struct asteroid_t *asteroid;
	struct bullet_t *bullet;

	s32 i;

	player = &state->player;

	printf("Player : (%3.2f, %3.2f)\n", player->movement.px, player->movement.py);

	for (i = 0; i < state->asteroids_len; i++) {
		asteroid = state->asteroids + i;

		if (!asteroid->is_used) continue;

		printf("Asteroid %d : (%3.2f, %3.2f)\n", i, asteroid->movement.px, asteroid->movement.py);
	}

	for (i = 0; i < ARRSIZE(state->bullets); i++) {
		bullet = state->bullets + i;

		if (!bullet->is_used) continue;

		printf("Bullet %d : (%3.2f, %3.2f)\n", i, bullet->movement.px, bullet->movement.py);
	}

	printf("\n");
}

// Update : the game update function
void Update(struct state_t *state)
{
	assert(state);

	UpdatePlayer(state);
	UpdateAsteroids(state);
	UpdateBullets(state);

	VerboseLogging(state);
}

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

		if (IsOOB(bullet->movement.px, bullet->movement.py, GAMERES_WIDTH, GAMERES_HEIGHT)) {
			bullet->is_used = 0;
		}

		if (!bullet->is_used)
			continue;

		// collision checks (this can potentially skip over some asteroids)
		for (j = 0; j < state->asteroids_len; j++) {
			asteroid = state->asteroids + j;
			if (!asteroid->is_used)
				continue;

			if (CheckBulletAsteroidCollision(state, i, j)) {
				bullet->is_used = 0;
				asteroid->is_used = 0;
				break;
			}
		}

		if (!bullet->is_used)
			continue;

		UpdateMovement(&state->bullets[i].movement);
	}
}

// CheckBulletAsteroidCollision : what it sounds like
s32 CheckBulletAsteroidCollision(struct state_t *state, s32 aidx, s32 bidx)
{
	struct bullet_t *bullet;
	struct asteroid_t *asteroid;
	struct asset_t *a_asteroid;
	SDL_Rect rect;
	vec2f curr;
	vec2f next;
	vec2f sides[4];

	assert(state);

	asteroid = state->asteroids + aidx;
	bullet = state->bullets + bidx;

	// we have to look up the dimensions of the asteroid

	a_asteroid = AssetFetchByName(&state->asset_container, "asteroid");

	rect.w = a_asteroid->w;
	rect.h = a_asteroid->h;
	rect.x = asteroid->movement.px - rect.w / 2;
	rect.y = asteroid->movement.py - rect.h / 2;

	sides[0].x = rect.x;
	sides[0].y = rect.y;

	sides[1].x = rect.x + rect.w;
	sides[1].y = rect.y;

	sides[2].x = rect.x + rect.w;
	sides[2].y = rect.y + rect.h;

	sides[3].x = rect.x + rect.w;
	sides[3].y = rect.y + rect.h;

	curr.x = bullet->movement.px;
	curr.y = bullet->movement.py;

	next.x = bullet->movement.px + bullet->movement.vx;
	next.y = bullet->movement.py + bullet->movement.vy;

#if 0
	return IsInside(sides, 4, curr) || WillCollide(sides, 4, curr, next);
#else
	return WillCollide(sides, 4, curr, next);
#endif
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

	// for (i = 0, n = RandInt(20, 30); i < n; i++) {
	for (i = 0, n = 1; i < n; i++) {
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

// IsOnSegment : returns true if q lies on the line segment pr
s32 IsOnSegment(vec2f p, vec2f q, vec2f r)
{
	// return (q.x <= MAX(p.x, r.x) && q.x >= MIN(p.x, r.x) &&
			// q.y <= MAX(p.y, r.y) && q.y >= MIN(p.y, r.y));
	return (MIN(p.x, r.x) <= q.x && q.x <= MAX(p.x, r.x)) &&
		   (MIN(p.y, r.y) <= q.y && q.y <= MAX(p.y, r.y));
}

// GetOrientation : returns the type of orientation
s32 GetOrientation(vec2f p, vec2f q, vec2f r)
{
	int v;

	// NOTE (Brian)
	//
	// for the orientation of the ordered triples, we return:
	//
	//    0 - p, q, and r are colinear
	//    1 - clockwise
	//   -1 - counterclockwise

	v = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

	if (v == 0) return 0;
	return v > 0 ? 1 : -1;
}

// DoesIntersect : returns true if the segments p1q1 and p2q2 intersect
s32 DoesIntersect(vec2f p1, vec2f q1, vec2f p2, vec2f q2)
{
	int orient[4];

	orient[0] = GetOrientation(p1, q1, p2);
	orient[1] = GetOrientation(p1, q1, q2);
	orient[2] = GetOrientation(p2, q2, p1);
	orient[3] = GetOrientation(p2, q2, q1);

	if (orient[0] != orient[1] && orient[2] != orient[3])
		return 1;

	if (orient[0] == 0 && IsOnSegment(p1, p2, q1))
		return 1;

	if (orient[1] == 0 && IsOnSegment(p1, q2, q1))
		return 1;

	if (orient[2] == 0 && IsOnSegment(p2, p1, q2))
		return 1;

	if (orient[3] == 0 && IsOnSegment(p2, q1, q2))
		return 1;

	return 0;
}

// IsInside : returns true if the point lays inside the polygon of n sides
s32 IsInside(vec2f *polygon, s32 n, vec2f point)
{
	s32 i, count, next;

	// NOTE (Brian) this is totally a hack, but it might just work
	vec2f extreme = { 2 * (polygon[1].x - polygon[0].x), point.y };

	if (n < 3)
		return 0;

	i = count = 0;

	do {
		next = (i + 1) % n;

		if (DoesIntersect(polygon[i], polygon[(i + 1) % n], point, extreme)) {
			if (GetOrientation(polygon[i], point, polygon[next]) == 0) {
				return IsOnSegment(polygon[i], point, polygon[next]);
			}

			count++;
		}

		i = next;

	} while (i != 0);

	return count % 2;
}

// WillCollide : returns true if the line segment created from curr -> next, will intersect with the polygon
s32 WillCollide(vec2f *polygon, s32 n, vec2f curr, vec2f next)
{
	s32 i;
	vec2f p1, p2;
	f32 t, u;
	f32 z;

	// NOTE (Brian) mostly stolen from here, but adapted to search for our rectangle
	//
	// https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	//
	// When you get back from your slurpee run, these are the things you'll need to do:
	//
	// 1. rename everything to match the stack overflow article, it's hard enough to know what's
	//    going on with that.
	// 2. program the rest of the owl
	// 3. return true on collision with the polygon's sides
	// 4. return false otherwise
	//
	// The lesson from the article is that you can take the cross product of two vectors and find
	// where they're supposed to intersect. There are some cases where the cross product is actually
	// zero, and in those cases, you basically do some special tests to really see if the lines
	// intersect.
	//
	// From the stack overflow example, p and r are the sides of our polygon, and q and s are 'curr'
	// and 'next.
	//
	// You'll have to check for the special cases BEFORE doing the math, otherwise you might get a
	// DIV BY ZERO error. This is why people use game engines.

	for (i = 0; i < n; i++) {
		p1 = polygon[(i + 0) % n];
		p2 = polygon[(i + 1) % n];

		z = Vec2fCross(Vec2fSub(curr, p1), next);

		t = z / Vec2fCross(p2, next);
		u = z / Vec2fCross(p2, next);
	}

	return 0;
}

f32 Vec2fCross(vec2f a, vec2f b)
{
	return a.x * b.y - a.y * b.x;
}

f32 Vec2fSub(vec2f a, vec2f b)
{
	vec2f c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return c;
}

