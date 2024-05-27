#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "display.h"
#include "window.h"
#include "buffer.h"
#include "buffer-png.h"
#include "pixel.h"
#include "drawing.h"
#include "operations.h"

static void demo_announce(char *name)
{
	printf("Starting demo: %s\n", name);
}

static void filter_kernel_normalize(float *kernel, unsigned int span)
{
	unsigned int i;
	float total = 0.;

	for (i = 0; i < span * span; i++)
		total += kernel[i];

	if (!total || total == 1.)
		return;

	for (i = 0; i < span * span; i++)
		kernel[i] /= total;
}

static char *drawings[] = {
	"rectangle",
	"rectangle-gradient",
	"disk",
	"disk-gradient",
	"lines-major-x",
	"lines-major-y",
	"lines",
	"circle",
	"circle-many-points",
	"circle-interpol",
	"butterfly",
	NULL
};

int demo_drawing(struct display *display, char *name)
{
	struct window *window = NULL;
	struct buffer *buffer = NULL;
	/* Buffer dimensions */
	unsigned int width = 1280;
	unsigned int height = 720;
	/* Line drawing parametric circle */
	unsigned int bound = width > height ? height : width;
	unsigned int lines = 16;
	float half_range = 3.1415;
	float step = 2. * half_range / lines;
	float theta = -half_range;
	float r = bound / 3.;
	unsigned int xc = width / 2;
	unsigned int yc = height / 2;
	/* Colors */
	uint32_t pixel_grey = pixel_pack(120, 120, 120, 255);
	uint32_t pixel_red = pixel_pack(255, 74, 62, 255);
	uint32_t pixel_green = pixel_pack(62, 255, 74, 255);
	uint32_t pixel_blue = pixel_pack(62, 74, 255, 255);

	unsigned int x, y;
	unsigned int i;
	int ret;

	/* Window */
	window = window_create(display, "drawing-operations", "Drawing demo");
	if (!window)
		goto error;

	/* Buffer */
	buffer = buffer_create(display, width, height);
	if (!buffer)
		goto error;

	demo_announce(name);

	draw_background(buffer, pixel_grey);

	if (!strcmp(name, "rectangle")) {
		draw_rectangle(buffer, width / 6, height / 6,
			       4 * width / 6, 4 * height / 6, pixel_red);
	} else if (!strcmp(name, "rectangle-gradient")) {
		draw_rectangle_gradient(buffer, width / 6, height / 6,
					4 * width / 6, 4 * height / 6,
					pixel_red, pixel_green);
	} else if (!strcmp(name, "disk")) {
		xc -= width / 4;

		draw_disk(buffer, xc, yc, r, pixel_red);

		xc += 2 * width / 4;
		yc += height / 4;

		draw_disk(buffer, xc, yc, r / 2., pixel_green);
	} else if (!strcmp(name, "disk-gradient")) {
		draw_disk_gradient(buffer, xc, yc, r, pixel_blue, pixel_green);
	} else if (!strcmp(name, "lines-major-x")) {
		for (i = 0; i < lines; i++) {
			x = xc + r * cosf(theta);
			y = yc + r * sinf(theta);

			draw_line_major_x(buffer, xc, yc, x, y, pixel_green);

			theta += step;
		}
	} else if (!strcmp(name, "lines-major-y")) {
		for (i = 0; i < lines; i++) {
			x = xc + r * cosf(theta);
			y = yc + r * sinf(theta);

			draw_line_major_y(buffer, xc, yc, x, y, pixel_green);

			theta += step;
		}
	} else if (!strcmp(name, "lines")) {
		for (i = 0; i < lines; i++) {
			x = xc + r * cosf(theta);
			y = yc + r * sinf(theta);

			draw_line(buffer, xc, yc, x, y, pixel_green);

			theta += step;
		}
	} else if (!strcmp(name, "circle")) {
		draw_circle(buffer, xc, yc, r, pixel_green, 200, false);
	} else if (!strcmp(name, "circle-many-points")) {
		draw_circle(buffer, xc, yc, r, pixel_green, 1000, false);
	} else if (!strcmp(name, "circle-interpol")) {
		draw_circle(buffer, xc, yc, r, pixel_green, 200, true);
	} else if (!strcmp(name, "butterfly")) {
		draw_butterfly(buffer, xc, yc, r, pixel_red, 250, true);
//		draw_butterfly(buffer, xc, yc, r * 2. / 3., pixel_green, 200, true);
//		draw_butterfly(buffer, xc, yc, r * 1. / 3., pixel_blue, 150, true);
	}

	ret = window_configured_wait(window);
	if (ret)
		goto error;

	/* Draw buffer to window. */
	ret = window_draw(window, buffer, NULL);
	if (ret)
		goto error;

	ret = window_closed_wait(window);
	if (ret)
		goto error;

	ret = 0;
	goto complete;

error:
	ret = -1;

complete:
	window_destroy(window);
	buffer_destroy(buffer);

	return ret;
}

static int demo_operation_scale(struct display *display,
				struct window *window_source, 
				struct window *window_destination,
				char *image_path, float factor)
{
	struct buffer *buffer_source = NULL;
	struct buffer *buffer_destination = NULL;
	unsigned int width, height;
	int ret;

	/* Source buffer from image. */
	buffer_source = buffer_create_from_png(display, image_path);
	if (!buffer_source)
		goto error;

	/* Scale source dimensions for destination buffer. */
	width = buffer_source->width * factor;
	height = buffer_source->height * factor;

	/* Desination buffer */
	buffer_destination = buffer_create(display, width, height);
	if (!buffer_destination)
		goto error;

	/* Scale source buffer to destination buffer. */
	operate_scaling(buffer_source, buffer_destination, factor);

	/* Wait for window configure callback. */
	ret = window_configured_wait(window_source);
	if (ret)
		goto error;

	/* Wait for window configure callback. */
	ret = window_configured_wait(window_destination);
	if (ret)
		goto error;

	/* Draw source buffer to source window. */
	ret = window_draw(window_source, buffer_source, NULL);
	if (ret)
		goto error;

	/* Draw destination buffer to destination window. */
	ret = window_draw(window_destination, buffer_destination, NULL);
	if (ret)
		goto error;

	/* Wait for window close callback. */
	ret = window_closed_wait(window_source);
	if (ret)
		goto error;

	/* Wait for window close callback. */
	ret = window_closed_wait(window_destination);
	if (ret)
		goto error;

	ret = 0;
	goto complete;

error:
	ret = -1;

complete:
	buffer_destroy(buffer_source);
	buffer_destroy(buffer_destination);

	return ret;
}

static int demo_operation_scale_alias(struct display *display,
				      struct window *window_source, 
				      struct window *window_destination,
				      char *image_path, float factor)
{
	struct buffer *buffer_source = NULL;
	struct buffer *buffer_destination = NULL;
	struct buffer *buffer_temporary = NULL;
	unsigned int width, height;
	int ret;

	/* Source buffer from image. */
	buffer_source = buffer_create_from_png(display, image_path);
	if (!buffer_source)
		goto error;

	/* Scale source dimensions for temporary buffer. */
	width = buffer_source->width * factor;
	height = buffer_source->height * factor;

	/* Temporary buffer for down-scaled result. */
	buffer_temporary = buffer_create(display, width, height);
	if (!buffer_temporary)
		goto error;

	/* Desination buffer */
	buffer_destination = buffer_create(display, buffer_source->width,
					   buffer_source->height);
	if (!buffer_destination)
		goto error;

	/* Down-scale source buffer to temporary buffer. */
	operate_scaling(buffer_source, buffer_temporary, factor);

	/* Up-scale temporary buffer to destination buffer. */
	operate_scaling(buffer_temporary, buffer_destination, 1. / factor);

	/* Wait for window configure callback. */
	ret = window_configured_wait(window_source);
	if (ret)
		goto error;

	/* Wait for window configure callback. */
	ret = window_configured_wait(window_destination);
	if (ret)
		goto error;

	/* Draw source buffer to source window. */
	ret = window_draw(window_source, buffer_source, NULL);
	if (ret)
		goto error;

	/* Draw destination buffer to destination window. */
	ret = window_draw(window_destination, buffer_destination, NULL);
	if (ret)
		goto error;

	/* Wait for window close callback. */
	ret = window_closed_wait(window_source);
	if (ret)
		goto error;

	/* Wait for window close callback. */
	ret = window_closed_wait(window_destination);
	if (ret)
		goto error;

	ret = 0;
	goto complete;

error:
	ret = -1;

complete:
	buffer_destroy(buffer_source);
	buffer_destroy(buffer_destination);
	buffer_destroy(buffer_temporary);

	return ret;
}

static int demo_operation_filter(struct display *display,
				 struct window *window_source, 
				 struct window *window_destination,
				 char *image_path, float *kernel,
				 unsigned int span)
{
	struct buffer *buffer_source = NULL;
	struct buffer *buffer_destination = NULL;
	unsigned int width, height;
	int ret;

	/* Source buffer from image. */
	buffer_source = buffer_create_from_png(display, image_path);
	if (!buffer_source)
		goto error;

	/* Keep source dimensions for destination buffer. */
	width = buffer_source->width;
	height = buffer_source->height;

	/* Desination buffer */
	buffer_destination = buffer_create(display, width, height);
	if (!buffer_destination)
		goto error;

	/* Filter source buffer to destination buffer. */
	operate_filter(buffer_source, buffer_destination, kernel, span);

	/* Wait for window configure callback. */
	ret = window_configured_wait(window_source);
	if (ret)
		goto error;

	/* Wait for window configure callback. */
	ret = window_configured_wait(window_destination);
	if (ret)
		goto error;

	/* Draw source buffer to source window. */
	ret = window_draw(window_source, buffer_source, NULL);
	if (ret)
		goto error;

	/* Draw destination buffer to destination window. */
	ret = window_draw(window_destination, buffer_destination, NULL);
	if (ret)
		goto error;

	/* Wait for window close callback. */
	ret = window_closed_wait(window_source);
	if (ret)
		goto error;

	/* Wait for window close callback. */
	ret = window_closed_wait(window_destination);
	if (ret)
		goto error;

	ret = 0;
	goto complete;

error:
	ret = -1;

complete:
	buffer_destroy(buffer_source);
	buffer_destroy(buffer_destination);

	return ret;
}

static char *operations[] = {
	"scale-down",
	"scale-up",
	"scale-alias",
	"filter-box-blur",
	"filter-gaussian",
	"filter-edge-detect",
	"filter-sharpen",
	NULL
};

int demo_operation(struct display *display, char *name)
{
	struct window *window_source = NULL;
	struct window *window_destination = NULL;
	int ret;

	/* Source window */
	window_source = window_create(display, "drawing-operations",
				      "Operation demo source");
	if (!window_source)
		goto error;

	/* Destination window */
	window_destination = window_create(display, "drawing-operations",
					   "Operation demo destination");
	if (!window_destination)
		goto error;

	demo_announce(name);

	if (!strcmp(name, "scale-down")) {
		ret = demo_operation_scale(display, window_source,
					   window_destination,
					   "samples/bbb-0.png", 0.5);
		if (ret)
			goto error;
	} else if (!strcmp(name, "scale-up")) {
		ret = demo_operation_scale(display, window_source,
					   window_destination,
					   "samples/bbb-1.png", 1.25);
		if (ret)
			goto error;
	} else if (!strcmp(name, "scale-alias")) {
		ret = demo_operation_scale_alias(display, window_source,
						 window_destination,
						 "samples/striped-dress.png",
						 0.05);
		if (ret)
			goto error;
	} else if (!strcmp(name, "filter-box-blur")) {
		float kernel[] = {
			1., 1., 1., 1., 1.,
			1., 1., 1., 1., 1.,
			1., 1., 1., 1., 1.,
			1., 1., 1., 1., 1.,
			1., 1., 1., 1., 1.,
		};
		unsigned int span = 5;

		filter_kernel_normalize(kernel, span);

		ret = demo_operation_filter(display, window_source,
					    window_destination,
					    "samples/carcassonne.png",
					    kernel, span);
		if (ret)
			goto error;
	} else if (!strcmp(name, "filter-gaussian")) {
		float kernel[] = {
			1.,  4.,  6.,  4., 1.,
			4., 16., 24., 16., 4.,
			6., 24., 36., 24., 6.,
			4., 16., 24., 16., 4.,
			1.,  4.,  6.,  4., 1.,
		};
		unsigned int span = 5;

		filter_kernel_normalize(kernel, span);

		ret = demo_operation_filter(display, window_source,
					    window_destination,
					    "samples/carcassonne.png",
					    kernel, span);
		if (ret)
			goto error;
	} else if (!strcmp(name, "filter-edge-detect")) {
		float kernel[] = {
			-1., -1., -1.,
			-1.,  8., -1.,
			-1., -1., -1.,
		};
		unsigned int span = 3;

		filter_kernel_normalize(kernel, span);

		ret = demo_operation_filter(display, window_source,
					    window_destination,
					    "samples/striped-dress.png",
					    kernel, span);
		if (ret)
			goto error;
	} else if (!strcmp(name, "filter-sharpen")) {
		float kernel[] = {
			 0., -1.,  0.,
			-1.,  5., -1.,
			 0., -1.,  0.,
		};
		unsigned int span = 3;

		filter_kernel_normalize(kernel, span);

		ret = demo_operation_filter(display, window_source,
					    window_destination,
					    "samples/striped-dress.png",
					    kernel, span);
		if (ret)
			goto error;
	}

	ret = 0;
	goto complete;

error:
	ret = -1;

complete:
	window_destroy(window_source);
	window_destroy(window_destination);

	return ret;
}

int demo(struct display *display, char *name)
{
	unsigned int i;

	for (i = 0; drawings[i]; i++)
		if (!strcmp(name, drawings[i]))
			return demo_drawing(display, name);

	for (i = 0; operations[i]; i++)
		if (!strcmp(name, operations[i]))
			return demo_operation(display, name);

	return -1;
}

void usage(void)
{
	unsigned int i;

	printf("drawing-operations [demo]\n");

	printf("\nAvailable drawing demos:\n");
	for (i = 0; drawings[i]; i++)
		printf("- %s\n", drawings[i]);

	printf("\nAvailable operation demos:\n");
	for (i = 0; operations[i]; i++)
		printf("- %s\n", operations[i]);
}

int main(int argc, char *argv[])
{
	struct display *display = NULL;
	char *name;
	int ret;

	if (argc < 2) {
		usage();
		return 0;
	}

	name = argv[1];

	display = display_create();
	if (!display)
		goto error;

	ret = demo(display, name);
	if (ret) {
		printf("Failed to start demo: %s\n", name);
		goto error;
	}

	ret = 0;
	goto complete;

error:
	ret = 1;

complete:
	display_destroy(display);

	return ret;
}
