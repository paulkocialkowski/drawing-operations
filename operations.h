#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

void operate_scaling(struct buffer *source, struct buffer *destination,
		     float factor);
void operate_filter(struct buffer *source, struct buffer *destination,
		    float *kernel, unsigned int span);

#endif
