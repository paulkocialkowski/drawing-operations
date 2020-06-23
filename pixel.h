#ifndef _PIXEL_H_
#define _PIXEL_H_

#include <stdint.h>

/* Adapted for ARGB8888 accessed from a little-endian CPU. */

static inline uint32_t pixel_pack(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint32_t color = 0;

	color |= b << 0;
	color |= g << 8;
	color |= r << 16;
	color |= a << 24;

	return color;
}

static inline uint8_t pixel_unpack_alpha(uint32_t color)
{
	return (color >> 24) & 0xff;
}

static inline uint8_t pixel_unpack_red(uint32_t color)
{
	return (color >> 16) & 0xff;
}

static inline uint8_t pixel_unpack_green(uint32_t color)
{
	return (color >> 8) & 0xff;
}

static inline uint8_t pixel_unpack_blue(uint32_t color)
{
	return (color >> 0) & 0xff;
}

#endif
