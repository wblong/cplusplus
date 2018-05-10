#ifndef __RINGBUFFER__H_
#define __RINGBUFFER__H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

struct ringbuffer;

/**
 * Create ringbuffer.
 *
 * @param memory        ring buffer memory address, function inner malloc 
                        memory when it is NULL.
 * @param block_length  per ring buffer block length.
 * @param block_count   block count.
 * @param read_pos      ring buffer read position address, function inner 
                        maintain when it is NULL.
 * @param write_pos     ring buffer write position address, function inner 
                        maintain when it is NULL.
 * @return 0 on success, -1 on parameter invalid, -2 on overflow, but still save data.
 */
struct ringbuffer *
ringbuffer_create(char *memory,
                        size_t block_length, 
                        size_t block_count,
                        int *read_pos,
                        int *write_pos);

void 
ringbuffer_delete(struct ringbuffer *rbuf);

/**
 * @return 0 on success, -1 on parameter invalid, -2 on overflow, but still save data.
 */
int 
ringbuffer_write(struct ringbuffer *rbuf, 
                    const char *buffer, 
                    size_t length);

/**
 * @return 0 on success, -1 on parameter invalid, -2 on ring buffer is empty.
 */
int 
ringbuffer_read(struct ringbuffer *rbuf, 
                    char *buffer, 
                    size_t length);

size_t
ringbuffer_get_length(struct ringbuffer *rbuf);

#ifdef __cplusplus
}
#endif
#endif