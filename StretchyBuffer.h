// #include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <ctype.h>

#define MAX(x, y) (((x) >= (y)) ? (x) : (y))

void *xrealloc(void *ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        perror("xrealloc failed");
        exit(1);
    }
    return ptr;
}

void *xmalloc(size_t num_bytes) {
    void *ptr = malloc(num_bytes);
    if (!ptr) {
        perror("malloc failed");
        exit(1);
    }
    return ptr;
}

/*
	NOTE: Will not compile in C++ because it requires pointer aliasing.
*/

typedef struct bufferHeader {
	size_t length;
	size_t cap;
	char buf[0];
} bufferHeader;

#define buf__hdr(b) ((bufferHeader *)((char *)b - offsetof(bufferHeader, buf)))   	 // Gives us a pointer to the buffer header
#define buf__fits(b, n) (buf_len(b) + n <= buf_cap(b)) 								 // Tells us whether we can fit n additional elements onto our buffer
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((b) = buf__grow((b), buf_len(b)+(n), sizeof(*(b))))) // Stretch the buffer to fit an additional n elements 

#define buf_len(b) ((b) ? (buf__hdr(b)->length) : 0)
#define buf_cap(b) ((b) ? (buf__hdr(b)->cap) : 0)
#define buf_push(b, x) (buf__fit(b,1)), (b[buf_len(b)] = (x), buf__hdr(b)->length++) // Stretch the buffer to accommodate an extra element, then push it on.
#define buf_free(b) ((b) ? free(buf__hdr(b)) : 0) 									 // Free the buffer (note that we need to free the buffer header too!)

void *buf__grow(const void *buffer, size_t new_length, size_t elem_size)
{
	size_t doubled_length = 1 + 2*buf_cap(buffer);
	size_t new_cap = MAX(doubled_length, new_length); // At least double the size of the buffer (this gives us "constant time" stretching)
	assert(new_length <= new_cap); // beware overflow
	size_t new_size = offsetof(bufferHeader, buf) + new_cap * elem_size;
	bufferHeader *new_header;
	if (buffer) {
		new_header = xrealloc(buf__hdr(buffer), new_size);
	} else {
		new_header = xmalloc(new_size);
		new_header->length = 0;	// Note we have to set the length otherwise we may start randomly pushing elements on at garbage locations. 
	}
	new_header->cap = new_cap;
	return new_header->buf;
}
