#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <wayland-client.h>

#include "display.h"
#include "window.h"
#include "region.h"

static void shell_surface_ping(void *data,
			       struct wl_shell_surface *shell_surface,
			       uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data,
				    struct wl_shell_surface *shell_surface,
				    uint32_t edges, int32_t width,
				    int32_t height)
{
}

static const struct wl_shell_surface_listener shell_surface_listener = {
	.ping = shell_surface_ping,
	.configure = shell_surface_configure,
};


struct window *window_create(struct display *display, char *title)
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

	window->shell_surface = wl_shell_get_shell_surface(display->shell,
							   window->surface);
	if (!window->shell_surface)
		goto error;

	wl_shell_surface_set_toplevel(window->shell_surface);
	wl_shell_surface_set_title(window->shell_surface, title);

	ret = wl_display_flush(display->display);
	if (ret < 0) {
		printf("Failed to flush Wayland display\n");
		goto error;
	}

	return window;

error:
	if (window) {
		if (window->shell_surface)
			wl_shell_surface_destroy(window->shell_surface);

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

	wl_shell_surface_destroy(window->shell_surface);
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
