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
	struct xdg_surface *xdg_surface;
	struct xdg_toplevel *xdg_toplevel;
	struct wl_callback *callback;

	bool configured;
	bool closed;
};

struct window *window_create(struct display *display, char *app_id,
			     char *title);
void window_destroy(struct window *window);
int window_draw(struct window *window, struct buffer *buffer, struct region *region);
int window_configured_wait(struct window *window);
int window_closed_wait(struct window *window);

#endif
