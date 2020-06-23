#ifndef _DRAWING_H_
#define _DRAWING_H_

void draw_background(struct buffer *buffer, uint32_t color);
void draw_rectangle(struct buffer *buffer, unsigned int x_start,
		    unsigned int y_start, unsigned int width,
		    unsigned int height, uint32_t color);
void draw_rectangle_gradient(struct buffer *buffer, unsigned int x_start,
			     unsigned int y_start, unsigned int width,
			     unsigned int height, uint32_t pixel_start,
			     uint32_t pixel_stop);
void draw_disk(struct buffer *buffer, unsigned int xc, unsigned int yc,
	       unsigned int radius, uint32_t color);
void draw_disk_gradient(struct buffer *buffer, unsigned int xc, unsigned int yc,
			unsigned int radius, uint32_t pixel_start,
			uint32_t pixel_stop);
void draw_line_major_x(struct buffer *buffer, unsigned int x_start,
		       unsigned int y_start, unsigned int x_stop,
		       unsigned int y_stop, uint32_t color);
void draw_line_major_y(struct buffer *buffer, unsigned int x_start,
		       unsigned int y_start, unsigned int x_stop,
		       unsigned int y_stop, uint32_t color);
void draw_line(struct buffer *buffer, unsigned int x_start,
	       unsigned int y_start, unsigned int x_stop, unsigned int y_stop,
	       uint32_t color);
void draw_circle(struct buffer *buffer, unsigned int xc, unsigned int yc,
		 unsigned int radius, uint32_t color, unsigned int points,
		 bool interpol);
void draw_butterfly(struct buffer *buffer, unsigned int xc, unsigned int yc,
		    unsigned int radius, uint32_t color, unsigned int points,
		    bool interpol);

#endif
