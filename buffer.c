#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include <wayland-client.h>

#include "display.h"
#include "window.h"
#include "region.h"

struct buffer *buffer_create(struct display *display, unsigned int width, unsigned int height)
{
	struct buffer *buffer = NULL;
	int ret;

	if (!display)
		goto error;

	buffer = calloc(1, sizeof(*buffer));
	if (!buffer)
		goto error;

	buffer->fd = -1;

	if (display->alpha_support)
		buffer->format = WL_SHM_FORMAT_ARGB8888;
	else
		buffer->format = WL_SHM_FORMAT_XRGB8888;

	buffer->display = display;
	buffer->width = width;
	buffer->height = height;
	buffer->stride = width * 4;
	buffer->size = buffer->stride * buffer->height;

	buffer->fd = memfd_create("wayland-buffer", MFD_CLOEXEC);
	if (buffer->fd < 0)
		goto error;

	ret = ftruncate(buffer->fd, buffer->size);
	if (ret)
		goto error;


	buffer->shm_pool = wl_shm_create_pool(display->shm, buffer->fd, buffer->size);
	if (!buffer->shm_pool)
		goto error;

	buffer->buffer = wl_shm_pool_create_buffer(buffer->shm_pool, 0,
						   buffer->width,
						   buffer->height,
						   buffer->stride,
						   buffer->format);
	if (!buffer->buffer)
		goto error;

	wl_shm_pool_destroy(buffer->shm_pool);
	buffer->shm_pool = NULL;

	buffer->data = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE,
			    MAP_SHARED, buffer->fd, 0);
	if (buffer->data == MAP_FAILED)
		goto error;

	return buffer;

error:
	if (buffer) {
		if (buffer->data && buffer->data != MAP_FAILED)
			munmap(buffer->data, buffer->size);

		if (buffer->buffer)
			wl_buffer_destroy(buffer->buffer);

		if (buffer->shm_pool)
			wl_shm_pool_destroy(buffer->shm_pool);

		if (buffer->fd >= 0)
			close(buffer->fd);

		free(buffer);
	}

	return NULL;
}

void buffer_destroy(struct buffer *buffer)
{
	if (!buffer)
		return;

	munmap(buffer->data, buffer->size);

	wl_buffer_destroy(buffer->buffer);

	close(buffer->fd);

	free(buffer);
}
