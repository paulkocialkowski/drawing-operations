#ifndef _BUFFER_PNG_H_
#define _BUFFER_PNG_H_

#include "buffer.h"
#include "display.h"

struct buffer *buffer_create_from_png(struct display *display, char *path);

#endif
