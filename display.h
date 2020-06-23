#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdbool.h>

#include <wayland-client.h>

struct display {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_output *output;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct wl_shell *shell;

	bool format_support;
	bool alpha_support;
};

struct display *display_create(void);
void display_destroy(struct display *display);

#endif
