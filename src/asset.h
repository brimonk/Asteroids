#ifndef ASSET_H
#define ASSET_H

/*
 *
 */

#include "common.h"

#include <SDL.h>

struct asset_t {
	void *bytes;
	SDL_Texture *texture;
	char *name;
	s32 w, h, c;
};

struct asset_container_t {
	struct asset_t *assets;
	size_t assets_len, assets_cap;
};

// function definition

// AssetLoad : loads a single asset from disk
s32 AssetLoad(struct asset_container_t *container, char *path);

// AssetsFree : releases all of the resources associated with the asset
s32 AssetsFree(struct asset_container_t *container);

#endif // ASSET_H

