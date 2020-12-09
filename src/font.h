#ifndef FONT_H
#define FONT_H

#include "common.h"
#include "stb_truetype.h"

// fontchar_t : a single character in a font
struct fontchar_t {
	u8 *bitmap; // rgba
	SDL_Texture *texture;
	// u8 *bitmap;
	u32 codepoint;
	s32 f_x; // font size (in pixels)
	s32 f_y;
	s32 b_x; // bearing information
	s32 b_y;
	s32 advance;
	s32 is_valid; // 1 for yes
};

// font_t : an entire font, rendered out
struct font_t {
	char *name;
	char *path;
	char *ttfbuffer;
	struct fontchar_t *fonttab;
	size_t fonttab_len, fonttab_cap;
	stbtt_fontinfo fontinfo;
	f32 scale_x, scale_y;
	s32 ascent;
	s32 descent;
	s32 linegap;
	s32 vertadvance;
	s32 size;
};

// FONT FUNCTIONS
// FontLoad : load + render the ttf file into the *font structure
s32 FontLoad(struct font_t *font, char *path, s32 size);

// FontLoadMetrics : load font metrics (init the library for the font)
s32 FontLoadMetrics(struct font_t *font, s32 size);

// FontLoadCodepoint : renders a unicode codepoint 
s32 FontLoadCodepoint(struct font_t *font, struct fontchar_t *fontchar, u32 codepoint);

// FontFree : free the font structure
s32 FontFree(struct font_t *font);

#endif // FONT_H

