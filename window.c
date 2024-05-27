#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include "display.h"
#include "window.h"
#include "region.h"

static void xdg_surface_configure(void *data, struct xdg_surface *surface,
				  uint32_t serial)
{
	xdg_surface_ack_configure(surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
	.configure = xdg_surface_configure,
};

static void xdg_toplevel_configure(void *data,
				   struct xdg_toplevel *xdg_toplevel,
				   int32_t width, int32_t height,
				   struct wl_array *states)
{
	struct window *window = data;

	window->configured = true;
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
	struct window *window = data;

	window->closed = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	.configure = xdg_toplevel_configure,
	.close = xdg_toplevel_close,
};

struct window *window_create(struct display *display, char *app_id, char *title)
{
	struct window *window = NULL;
	int ret;

	if (!display)
		goto error;

	window = calloc(1, sizeof(*window));
	if (!window)
		goto error;

	window->display = display;

	window->surface = wl_compositor_create_surface(display->compositor);
	if (!window->surface)
		goto error;

	window->xdg_surface = xdg_wm_base_get_xdg_surface(display->xdg_shell,
							  window->surface);
	if (!window->xdg_surface)
		goto error;

	xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener,
				 window);

	window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
	if (!window->xdg_toplevel)
		goto error;

	xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener,
				  window);

	xdg_toplevel_set_app_id(window->xdg_toplevel, app_id);
	xdg_toplevel_set_title(window->xdg_toplevel, title);

	wl_surface_commit(window->surface);

	ret = wl_display_flush(display->display);
	if (ret < 0) {
		printf("Failed to flush Wayland display\n");
		goto error;
	}

	return window;

error:
	if (window) {
		if (window->xdg_toplevel)
			xdg_toplevel_destroy(window->xdg_toplevel);

		if (window->xdg_surface)
			xdg_surface_destroy(window->xdg_surface);

		if (window->surface)
			wl_surface_destroy(window->surface);

		free(window);
	}

	return NULL;
}

void window_destroy(struct window *window)
{
	if (!window)
		return;

	xdg_toplevel_destroy(window->xdg_toplevel);
	xdg_surface_destroy(window->xdg_surface);
	wl_surface_destroy(window->surface);

	free(window);
}

int window_draw(struct window *window, struct buffer *buffer, struct region *region)
{
	struct region region_full = { 0 };
	int ret;

	if (!window || !buffer)
		return -EINVAL;

	if (!region) {
		region_full.x = 0;
		region_full.y = 0;
		region_full.width = buffer->width;
		region_full.height = buffer->height;

		region = &region_full;
	}

	wl_surface_attach(window->surface, buffer->buffer, 0, 0);
	wl_surface_damage(window->surface, region->x, region->y, region->width,
			  region->height);

	wl_surface_commit(window->surface);

	ret = wl_display_flush(buffer->display->display);
	if (ret < 0) {
		printf("Failed to flush Wayland display\n");
		ret = -errno;
		goto error;
	}

	return 0;

error:
	return ret;
}

int window_configured_wait(struct window *window)
{
	struct display *display = window->display;
	int ret;

	while (!window->configured) {
		ret = wl_display_dispatch(display->display);
		if (ret < 0)
			return ret;
	}

	return 0;
}

int window_closed_wait(struct window *window)
{
	struct display *display = window->display;
	int ret;

	while (!window->closed) {
		ret = wl_display_dispatch(display->display);
		if (ret < 0)
			return ret;
	}

	return 0;
}
