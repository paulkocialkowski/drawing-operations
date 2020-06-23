#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <wayland-client.h>
#include <cairo.h>

#include "display.h"
#include "buffer.h"
#include "buffer-png.h"

struct buffer *buffer_create_from_png(struct display *display, char *path)
{
	cairo_surface_t *surface = NULL;
	struct buffer *buffer = NULL;
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	unsigned int y;
	void *data;

	surface = cairo_image_surface_create_from_png(path);
	if (!surface)
		goto error;

	data = cairo_image_surface_get_data(surface);
	width = cairo_image_surface_get_width(surface);
	height = cairo_image_surface_get_height(surface);
	stride = cairo_image_surface_get_stride(surface);

	buffer = buffer_create(display, width, height);

	for (y = 0; y < height; y++)
		memcpy(buffer->data + y * buffer->stride,
		       data + y * stride, width * sizeof(uint32_t));

	cairo_surface_destroy(surface);

	return buffer;

error:
	if (surface)
		cairo_surface_destroy(surface);

	if (buffer)
		buffer_destroy(buffer);

	return NULL;
}
