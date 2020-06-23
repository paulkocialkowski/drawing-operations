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

/* Implements nearest-neighbour scaling. */
void operate_scaling(struct buffer *source, struct buffer *destination,
		     float factor)
{
	unsigned int width, height;
	unsigned int x, y;

	width = destination->width;
	height = destination->height;

	/* Iterate over destination dimensions. */
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			uint32_t *pixel_src, *pixel_dst;

			/* Source coordinates are calculated for each
			 * destination coordinates. */
			pixel_src = buffer_pixel(source, x / factor, y / factor);

			/* Destination coordinates are iterated directly. */
			pixel_dst = buffer_pixel(destination, x, y);

			/* Affect the pixel value. */
			*pixel_dst = *pixel_src;
		}
	}
}

/* Straightforward linear filtering. */
void operate_filter(struct buffer *source, struct buffer *destination,
		    float *kernel, unsigned int span)
{
	unsigned int half_span = span / 2;
	unsigned int width, height;
	unsigned int x, y;

	/* Sanity check: source and destination dimensions must be equal. */
	if (source->width != destination->width ||
	    source->height != destination->height)
		return;

	/* The current pixel needs to be at the center. */
	if (!(span % 2))
		return;

	width = source->width;
	height = source->height;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			/* Per-axis kernel beginning index. */
			unsigned int xfb = 0, yfb = 0;
			/* Per-axis kernel end bound. */
			unsigned int xfe = span, yfe = span;
			/* Per-axis kernel coordinates. */
			unsigned int xf, yf;
			/* Accumulators for each channel value. */
			float rf = 0;
			float gf = 0;
			float bf = 0;
			uint32_t *pixel;

			/* Border conditions to restrict the filter to existing
			 * pixels only. Better approaches do exist. */

			if (x < half_span)
				xfb = half_span - x;
			if ((width - x - 1) < half_span)
				xfe = half_span + x - width + 1;

			if (y < half_span)
				yfb = half_span - y;
			if ((height - y - 1) < half_span)
				yfe = half_span + y - height + 1;

			/* Iterate over the applicable kernel elements. */
			for (yf = yfb; yf < yfe; yf++) {
				for (xf = xfb; xf < xfe; xf++) {
					uint32_t color;
					uint8_t rc, gc, bc;
					/* Corresponding kernel factor. */
					float factor = kernel[yf * span + xf];
					/* Final source coordinates. */
					unsigned int xs, ys;

					/* The pixel at coordinates (x,y) is
					 * paired with the center of the kernel,
					 * which corresponds to:
					 * xf = yf = half_span. */
					/* Calculate the source coordinates for
					 * the paired kernel factor as such. */
					xs = x + xf - half_span;
					ys = y + yf - half_span;

					/* Get the color of the pixel. */
					color = *buffer_pixel(source, xs, ys);

					/* Unpack each component. */
					rc = pixel_unpack_red(color);
					gc = pixel_unpack_green(color);
					bc = pixel_unpack_blue(color);

					/* Weigh each channel by the kernel
					 * factor */
					rf += rc * factor;
					gf += gc * factor;
					bf += bc * factor;
				}
			}

			/* Sanity checks for float-to-uint8_t casting. */

			if (rf > 255.)
				rf = 255.;
			else if (rf < 0.)
				rf = 0.;

			if (gf > 255.)
				gf = 255.;
			else if (gf < 0.)
				gf = 0.;

			if (bf > 255.)
				bf = 255.;
			else if (bf < 0.)
				bf = 0.;

			/* Store the filtered pixel value at the destination. */
			pixel = buffer_pixel(destination, x, y);
			*pixel = pixel_pack(rf, gf, bf, 255);
		}
	}
}
