/*
 * Brian Chrzanowski
 * 2020-12-31 01:53:34
 *
 * Asset Handling System
 */

#include "common.h"

#include "stb_image.h"

#include "asset.h"

extern SDL_Renderer *gRenderer;

// AssetLoad : loads a single asset from disk
s32 AssetLoad(struct asset_container_t *container, char *path)
{
	SDL_Surface *surface;
	s32 x, y, n;
	u32 rmask, gmask, bmask, amask;
	struct asset_t *asset;

	C_RESIZE(&container->assets);

	asset = container->assets + container->assets_len++;

	// get bytes from the image itself off of disk
	asset->bytes = stbi_load(path, &x, &y, &n, 4);
	if (!asset->bytes) {
		ERR("COULDN'T LOAD IMAGE '%s'\n", path);
		return -1;
	}

	asset->w = x;
	asset->h = y;
	asset->c = n;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	// then make an SDL surface for it
	surface = SDL_CreateRGBSurfaceFrom(asset->bytes, x, y, 32, 4 * x, rmask, gmask, bmask, amask);
	if (surface == NULL) {
		ERR("SDL_CreateRGBSurface failed for '%s': %s\n", path, SDL_GetError());
		return -1;
	}

	// then create a texture from the surface
	asset->texture = SDL_CreateTextureFromSurface(gRenderer, surface);
	if (asset->texture == NULL) {
		ERR("SDL_CreateTextureFromSurface failed for '%s': %s\n", path, SDL_GetError());
		return -1;
	}

	// asset->name = strdup_null(strrchr(path, '/') + 1);
	asset->name = strslice(path, strrchr(path, '/') - path + 1, strrchr(path, '.') - path);

	return 0;
}

// AssetFetchByName : fetches an asset by name
struct asset_t *AssetFetchByName(struct asset_container_t *container, char *name)
{
	struct asset_t *asset;
	s32 i;

	assert(name);

	for (i = 0, asset = NULL; i < container->assets_len; i++) {
		asset = container->assets + i;
		if (streq(name, asset->name)) {
			return asset;
		}
	}

	return NULL;
}

// AssetsFree : releases all of the resources associated with the asset
s32 AssetsFree(struct asset_container_t *container)
{
	struct asset_t *asset;
	s32 i;

	for (i = 0; i < container->assets_len; i++) {
		asset = container->assets + i;
		SDL_DestroyTexture(asset->texture);
		stbi_image_free(asset->bytes);
		free(asset->name);
	}

	return 0;
}

