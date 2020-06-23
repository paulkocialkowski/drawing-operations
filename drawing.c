#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>

#include <sys/types.h>

#include "display.h"
#include "window.h"
#include "buffer.h"
#include "pixel.h"

void draw_background(struct buffer *buffer, uint32_t color)
{
	wmemset(buffer->data, color, buffer->size / sizeof(color));
}

void draw_rectangle(struct buffer *buffer, unsigned int x_start,
		    unsigned int y_start, unsigned int width,
		    unsigned int height, uint32_t color)
{
	unsigned int x_stop = x_start + width;
	unsigned int y_stop = y_start + height;
	unsigned int x, y;
	uint32_t *pixel;

	for (y = y_start; y < y_stop; y++) {
		for (x = x_start; x < x_stop ; x++) {
			pixel = buffer_pixel(buffer, x, y);
			*pixel = color;
		}
		//wmemset(pixel, color, width);
	}
}

void draw_rectangle_gradient(struct buffer *buffer, unsigned int x_start,
			     unsigned int y_start, unsigned int width,
			     unsigned int height, uint32_t pixel_start,
			     uint32_t pixel_stop)
{
	unsigned int x_stop = x_start + width;
	unsigned int y_stop = y_start + height;
	unsigned int x, y;
	uint32_t *pixel;

	uint8_t rs = pixel_unpack_red(pixel_start);
	uint8_t re = pixel_unpack_red(pixel_stop);
	uint8_t gs = pixel_unpack_green(pixel_start);
	uint8_t ge = pixel_unpack_green(pixel_stop);
	uint8_t bs = pixel_unpack_blue(pixel_start);
	uint8_t be = pixel_unpack_blue(pixel_stop);

	float sr = (re - rs) / (float)width;
	float sg = (ge - gs) / (float)width;
	float sb = (be - bs) / (float)width;

	for (y = y_start; y < y_stop; y++) {
		for (x = x_start; x < x_stop; x++) {
			uint8_t r = rs + (x - x_start) * sr;
			uint8_t g = gs + (x - x_start) * sg;
			uint8_t b = bs + (x - x_start) * sb;

			pixel = buffer_pixel(buffer, x, y);
			*pixel = pixel_pack(r, g, b, 255);
		}
	}
}

void draw_disk(struct buffer *buffer, unsigned int xc, unsigned int yc,
	       unsigned int radius, uint32_t color)
{
	unsigned int x, y;

	unsigned int x_start = xc - radius;
	unsigned int x_stop = xc + radius;
	unsigned int y_start = yc - radius;
	unsigned int y_stop = yc + radius;
	uint32_t *pixel;
	float r;

	if (xc < radius || yc < radius)
		return;

	for (y = y_start; y < y_stop; y++) {
		for (x = x_start; x < x_stop; x++) {
			r = sqrtf(pow((int)x - (int)xc, 2) + pow((int)y - (int)yc, 2));

			if (r > radius)
				continue;

			pixel = buffer_pixel(buffer, x, y);
			*pixel = color;
		}
	}
}

void draw_disk_gradient(struct buffer *buffer, unsigned int xc, unsigned int yc,
			unsigned int radius, uint32_t pixel_start,
			uint32_t pixel_stop)
{
	unsigned int x, y;
	uint32_t *pixel;
	float r;

	unsigned int x_start = xc - radius;
	unsigned int x_stop = xc + radius;
	unsigned int y_start = yc - radius;
	unsigned int y_stop = yc + radius;

	uint8_t rs = pixel_unpack_red(pixel_start);
	uint8_t re = pixel_unpack_red(pixel_stop);
	uint8_t gs = pixel_unpack_green(pixel_start);
	uint8_t ge = pixel_unpack_green(pixel_stop);
	uint8_t bs = pixel_unpack_blue(pixel_start);
	uint8_t be = pixel_unpack_blue(pixel_stop);

	float sr = (re - rs) / (float)radius;
	float sg = (ge - gs) / (float)radius;
	float sb = (be - bs) / (float)radius;

	if (xc < radius || yc < radius)
		return;

	for (y = y_start; y < y_stop; y++) {
		for (x = x_start; x < x_stop; x++) {
			float rr = sqrtf(pow((int)x - (int)xc, 2) + pow((int)y - (int)yc, 2));

			if (rr > radius)
				continue;

			uint8_t r = rs + rr * sr;
			uint8_t g = gs + rr * sg;
			uint8_t b = bs + rr * sb;

			pixel = buffer_pixel(buffer, x, y);
			*pixel = pixel_pack(r, g, b, 255);
		}
	}
}

void draw_line_major_x(struct buffer *buffer, unsigned int x_start,
		       unsigned int y_start, unsigned int x_stop,
		       unsigned int y_stop, uint32_t color)
{
	unsigned int x, y;
	float slope;

	slope = ((float)y_stop - (float)y_start) /
		((float)x_stop - (float)x_start);

	if (x_start > x_stop) {
		unsigned int xt, yt;

		xt = x_start;
		yt = y_start;
		x_start = x_stop;
		y_start = y_stop;
		x_stop = xt;
		y_stop = yt;
	}

	for (x = x_start; x < x_stop; x++) {
		uint32_t *pixel;

		y = y_start + (x - x_start) * slope;

		pixel = buffer_pixel(buffer, x, y);
		*pixel = color;
	}
}

void draw_line_major_y(struct buffer *buffer, unsigned int x_start,
		       unsigned int y_start, unsigned int x_stop,
		       unsigned int y_stop, uint32_t color)
{
	unsigned int x, y;
	float slope;

	slope = ((float)y_stop - (float)y_start) /
		((float)x_stop - (float)x_start);

	if (y_start > y_stop) {
		unsigned int xt, yt;

		xt = x_start;
		yt = y_start;
		x_start = x_stop;
		y_start = y_stop;
		x_stop = xt;
		y_stop = yt;
	}

	for (y = y_start; y < y_stop; y++) {
		uint32_t *pixel;

		x = x_start + (y - y_start) / slope;

		pixel = buffer_pixel(buffer, x, y);
		*pixel = color;
	}
}

void draw_line(struct buffer *buffer, unsigned int x_start,
	       unsigned int y_start, unsigned int x_stop, unsigned int y_stop,
	       uint32_t color)
{
	draw_line_major_x(buffer, x_start, y_start, x_stop, y_stop, color);
	draw_line_major_y(buffer, x_start, y_start, x_stop, y_stop, color);
}

void draw_circle(struct buffer *buffer, unsigned int xc, unsigned int yc,
		 unsigned int radius, uint32_t color, unsigned int points,
		 bool interpol)
{
	unsigned int x,y;
	float half_range = 3.1415;
	float theta = -half_range;
	float step = 2. * half_range / points; 
	unsigned int i;
	uint32_t *pixel;
	unsigned int xp, yp;
	unsigned int xs, ys;

	for (i = 0; i < points; i++) {
		x = xc + radius * cosf(theta);
		y = yc + radius * sinf(theta);

		pixel = buffer_pixel(buffer, x, y);
		*pixel = color;

		theta += step;

		if (interpol) {
			if (i > 0) {
				draw_line(buffer, xp, yp, x, y, color);
			} else {
				xs = x;
				ys = y;
			}

			xp = x;
			yp = y;
		}
	}

	if (interpol)
		draw_line(buffer, xs, ys, x, y, color);
}

void draw_butterfly(struct buffer *buffer, unsigned int xc, unsigned int yc,
		    unsigned int radius, uint32_t color, unsigned int points,
		    bool interpol)
{
	float pi = 3.1415;
	float half_range = pi;
	float theta = -half_range;
	float step = 2. * half_range / points; 
	unsigned int xp, yp;
	unsigned int xs, ys;
	unsigned int x,y;
	uint32_t *pixel;
	unsigned int i;

	for (i = 0; i < points; i++) {
		float r = (expf(sinf(theta)) - 2. * cosf(4. * theta) +
			  powf(sin((2. * theta - pi) / 24.), 4.)) / -3.;

		x = xc + radius * r * cosf(theta);
		y = yc + radius * r * sinf(theta);

		pixel = buffer_pixel(buffer, x, y);
		*pixel = color;

		theta += step;

		if (interpol) {
			if (i > 0) {
				draw_line(buffer, xp, yp, x, y, color);
			} else {
				xs = x;
				ys = y;
			}

			xp = x;
			yp = y;
		}
	}

	if (interpol)
		draw_line(buffer, xs, ys, x, y, color);
}
