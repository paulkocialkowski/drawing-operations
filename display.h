#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdbool.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

struct display {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_output *output;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct xdg_wm_base *xdg_shell;

	bool format_support;
	bool alpha_support;
};

struct display *display_create(void);
void display_destroy(struct display *display);

#endif
