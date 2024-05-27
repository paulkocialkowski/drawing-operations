#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include "display.h"

/* Output */

static void output_geometry(void *data, struct wl_output *output, int32_t x,
			    int32_t y, int32_t physical_width,
			    int32_t physical_height, int32_t subpixel,
			    const char *make, const char *model,
			    int32_t transform)
{
	printf("Output update:\n");
	printf("- Position: %dx%d (px)\n", x, y);
	printf("- %sransformed coordinates\n", transform ? "T" : "Unt");
	printf("- Device: %s %s\n", make, model);
	printf("- Physical size: %dx%d (mm)\n", physical_width, physical_height);
	printf("\n");
}

static void output_mode(void *data, struct wl_output *output, uint32_t flags,
			int32_t width, int32_t height, int32_t refresh_rate)
{
	struct chill_display_mode *mode = data;

	printf("Output mode update:\n");
	printf("- Dimensions: %dx%d (px)\n", width, height);
	printf("- Refresh rate: %.3f Hz\n", refresh_rate, refresh_rate / 1000.);
}

static void output_done(void *data, struct wl_output *output)
{
}

static void output_scale(void *data, struct wl_output *output, int32_t factor)
{
	printf("- Scaling factor: %d\n", factor);
	printf("\n");
}

static void output_name(void *data, struct wl_output *output, const char *name)
{
}

static void output_description(void *data, struct wl_output *output,
			       const char *description)
{
}

static const struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.done = output_done,
	.scale = output_scale,
	.name = output_name,
	.description = output_description,
};

/* SHM */

static void shm_format(void *data, struct wl_shm *shm, uint32_t format)
{
	struct display *display = data;

	if (format == WL_SHM_FORMAT_XRGB8888) {
		display->format_support = true;
	} else if (format == WL_SHM_FORMAT_ARGB8888) {
		display->format_support = true;
		display->alpha_support = true;
	}
}

static const struct wl_shm_listener shm_listener = {
	shm_format,
};

/* XDG Shell */

static void xdg_shell_ping(void *data, struct xdg_wm_base *xdg_shell,
			     uint32_t serial)
{
	xdg_wm_base_pong(xdg_shell, serial);
}

static const struct xdg_wm_base_listener xdg_shell_listener = {
	.ping = xdg_shell_ping,
};

/* Registry */

static void registry_global(void *private, struct wl_registry *registry,
			    uint32_t id, const char *interface,
			    uint32_t version)
{
	struct display *display = private;

	if (!strcmp(interface, wl_output_interface.name))
		display->output = wl_registry_bind(registry, id,
						   &wl_output_interface,
						   version);
	else if (!strcmp(interface, wl_compositor_interface.name))
		display->compositor = wl_registry_bind(registry, id,
						       &wl_compositor_interface,
						       version);
	else if (!strcmp(interface, wl_shm_interface.name))
		display->shm = wl_registry_bind(registry, id,
						&wl_shm_interface,
						version);
	else if (!strcmp(interface, xdg_wm_base_interface.name))
		display->xdg_shell = wl_registry_bind(registry, id,
						      &xdg_wm_base_interface,
						      3);
}

static void registry_global_remove(void *private, struct wl_registry *registry,
				   uint32_t id)
{
}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_global_remove,
};

/* Display */

struct display *display_create(void)
{
	struct display *display = NULL;
	int ret;

	display = calloc(1, sizeof(*display));
	if (!display)
		goto error;

	display->display = wl_display_connect(NULL);
	if (!display->display)
		goto error;

	display->registry = wl_display_get_registry(display->display);
	if (!display->registry)
		goto error;

	ret = wl_registry_add_listener(display->registry, &registry_listener, display);
	if (ret)
		goto error;

	wl_display_dispatch(display->display);

	if (!display->output || !display->compositor || !display->shm ||
	    !display->xdg_shell)
		goto error;

	ret = wl_output_add_listener(display->output, &output_listener, display);
	if (ret)
		goto error;

	ret = wl_shm_add_listener(display->shm, &shm_listener, display);
	if (ret)
		goto error;

	ret = xdg_wm_base_add_listener(display->xdg_shell, &xdg_shell_listener,
				       display);
	if (ret)
		goto error;

	ret = wl_display_roundtrip(display->display);
	if (ret < 0)
		goto error;

	if (!display->format_support) {
		printf("Unsupported format for Wayland\n");
		goto error;
	}

	return display;

error:
	if (display) {
		if (display->output)
			wl_output_destroy(display->output);

		if (display->compositor)
			wl_compositor_destroy(display->compositor);

		if (display->shm)
			wl_shm_destroy(display->shm);

		if (display->xdg_shell)
			xdg_wm_base_destroy(display->xdg_shell);

		if (display->display)
			wl_display_disconnect(display->display);

		free(display);
	}

	return NULL;
}

void display_destroy(struct display *display)
{
	if (!display)
		return;

	wl_output_destroy(display->output);
	wl_compositor_destroy(display->compositor);
	wl_shm_destroy(display->shm);
	xdg_wm_base_destroy(display->xdg_shell);

	wl_display_disconnect(display->display);

	free(display);
}
