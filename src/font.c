// ////////////////////////////////////////////////////////////////////////////
// BRIAN CHRZANOWSKI
// 2020-12-05 02:12:51
//
// Font Functions
// ////////////////////////////////////////////////////////////////////////////

#include <SDL.h>

#include "common.h"

#include "font.h"

extern SDL_Renderer *gRenderer;

// FontLoad : load + render the ttf file into the *font structure
s32 FontLoad(struct font_t *font, char *path, s32 size)
{
	s32 i;
	s32 rc;

	assert(font);

	font->path = strdup_null(path);
	font->ttfbuffer = sys_readfile(path);

	FontLoadMetrics(font, size);

	for (i = 0; i < 0xff; i++) { // load all 'ascii-ish'
		C_RESIZE(&font->fonttab);
		rc = FontLoadCodepoint(font, &font->fonttab[i], i);
		font->fonttab_len++;
	}

	return 0;
}

// FontLoadMetrics : load font metrics (init the library for the font)
s32 FontLoadMetrics(struct font_t *font, s32 size)
{
	s32 rc;

	assert(font);

	rc = stbtt_InitFont(&font->fontinfo, (unsigned char *)font->ttfbuffer, stbtt_GetFontOffsetForIndex((unsigned char *)font->ttfbuffer, 0));
	if (rc == 0) {
		ERR("Couldn't initialize font!\n");
		return -1;
	}

	// NOTE (Brian) do we need to scale our fonts differently????
	font->size = size;
	font->scale_y = stbtt_ScaleForPixelHeight(&font->fontinfo, size);
	font->scale_x = font->scale_y;

	stbtt_GetFontVMetrics(&font->fontinfo, &font->ascent, &font->descent, &font->linegap);

	font->ascent  *= font->scale_y;
	font->descent *= font->scale_y;
	font->linegap *= font->scale_y;

	font->vertadvance = font->ascent - font->descent + font->linegap;

	return 0;
}

// FontLoadCodepoint : renders a unicode codepoint 
s32 FontLoadCodepoint(struct font_t *font, struct fontchar_t *fontchar, u32 codepoint)
{
	s32 w, h, xoff, yoff;
	s32 i;
	u8 *bitmap;

	assert(font);
	assert(fontchar);

	memset(fontchar, 0, sizeof(*fontchar));

	bitmap = stbtt_GetCodepointBitmap(&font->fontinfo, font->scale_x, font->scale_y, codepoint, &w, &h, &xoff, &yoff);
	if (codepoint != ' ' && !bitmap) {
		return -1;
	}

	fontchar->is_valid = 1;

	stbtt_GetCodepointHMetrics(&font->fontinfo, (int)codepoint, &fontchar->advance, NULL);

	fontchar->codepoint = codepoint;

	// convert between the alpha bitmap, and a white with alpha texture for the font
	fontchar->bitmap = calloc(w * h, sizeof(u32));

	// NOTE (Brian) there's some funkiness about the byte ordering on this, and how the byte
	// ordering is in my head. I _originally_ had the reverse; of this, and there was all kinds of
	// nonsense.
	for (i = 0; i < w * h; i++) {
		fontchar->bitmap[i * 4 + 3] = 0xff;
		fontchar->bitmap[i * 4 + 2] = 0xff;
		fontchar->bitmap[i * 4 + 1] = 0xff;
		fontchar->bitmap[i * 4 + 0] = bitmap[i];
	}

	fontchar->f_x      = w;
	fontchar->f_y      = h;
	fontchar->b_x      = xoff;
	fontchar->b_y      = yoff;
	fontchar->advance *= font->scale_x;

	// now, ensure we can use it from SDL
	fontchar->texture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, w, h);
	if (fontchar->texture == NULL) {
		ERR("Couldn't render char 0x%X to font texture: %s", SDL_GetError());
		return -1;
	}

	SDL_SetTextureColorMod(fontchar->texture, 0xff, 0xff, 0xff);
	SDL_SetTextureBlendMode(fontchar->texture, SDL_BLENDMODE_BLEND);

	s32 rc;
	rc = SDL_UpdateTexture(fontchar->texture, NULL, fontchar->bitmap, 4 * w);
	if (rc < 0) {
		ERR("Couldn't output do the thing: %s\n", SDL_GetError());
	}

	return 0;
}

// FontFree : free the font structure
s32 FontFree(struct font_t *font)
{
	assert(font);

	// TODO (brian) actually free fonts :P

	return 0;
}

