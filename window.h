#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <stdbool.h>

#include <wayland-client.h>

#include "display.h"
#include "buffer.h"
#include "region.h"

struct window {
	struct display *display;

	struct wl_surface *surface;
	struct wl_shell_surface *shell_surface;
	struct wl_callback *callback;
};

struct window *window_create(struct display *display, char *title);
void window_destroy(struct window *window);
int window_draw(struct window *window, struct buffer *buffer, struct region *region);

#endif
