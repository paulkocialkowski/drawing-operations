#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <stdbool.h>

#include <wayland-client.h>

struct buffer {
	struct display *display;

	struct wl_shm_pool *shm_pool;
	struct wl_buffer *buffer;

	uint32_t format;
	unsigned int width;
	unsigned int height;
	unsigned int stride;

	void *data;
	unsigned int size;

	int fd;
};

static inline uint32_t *buffer_pixel(struct buffer *buffer, unsigned int x,
				     unsigned int y)
{
	unsigned int offset = y * buffer->stride + x * sizeof(uint32_t);

	return (uint32_t *)(buffer->data + offset);
}

struct buffer *buffer_create(struct display *display, unsigned int width, unsigned int height);
void buffer_destroy(struct buffer *buffer);

#endif
