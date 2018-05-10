#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"

#define rbuf_pos(p)   (*p)

struct ringbuffer {
    /* buffer memory address */
    char *memory;
    /* 1 indicate internal allocate, 0 indicate caller transfer it */
    int malloced;

    /* per block space length */
    size_t block_length;
    /* block max count */
    size_t block_count;

    int *r_pos;
    int *w_pos;
    size_t ring_length;

    int local_read_pos;
    int local_write_pos;
};
//!_rbuf_pos_increament
static int 
_rbuf_pos_increment(int *pos, size_t max)
{
    rbuf_pos(pos) = (rbuf_pos(pos) + 1) % max;
    return rbuf_pos(pos);
}

struct ringbuffer *
ringbuffer_create(char *memory,
                        size_t block_length, 
                        size_t block_count,
                        int *read_pos,
                        int *write_pos)
{
    struct ringbuffer *rbuf = (struct ringbuffer *) calloc(1, sizeof(*rbuf));

    if (rbuf) {
        rbuf->block_length = block_length;
        rbuf->block_count = block_count+1;
        rbuf->malloced = !memory;

        rbuf->r_pos = read_pos ? read_pos : &rbuf->local_read_pos;
        rbuf->w_pos = write_pos ? write_pos : &rbuf->local_write_pos;
        rbuf->ring_length = 0;
        rbuf_pos(rbuf->r_pos) = rbuf_pos(rbuf->w_pos) = 0;

        rbuf->memory = memory ? memory : (char *) malloc(block_length * block_count);
        if (!rbuf->memory) {
            free(rbuf);
            rbuf = NULL;
        }
    }
    return rbuf;
}

void 
ringbuffer_delete(struct ringbuffer *rbuf)
{
    if (rbuf) {
        if (rbuf->malloced && rbuf->memory)
            free(rbuf->memory);
        free(rbuf);
    }
}

int 
ringbuffer_write(struct ringbuffer *rbuf, 
                    const char *buffer, 
                    size_t length)
{
    int ret = 0;

    if (length > rbuf->block_length)
        return -1;

    memmove(rbuf->memory + rbuf->block_length*rbuf_pos(rbuf->w_pos), buffer, length);
    _rbuf_pos_increment(rbuf->w_pos, rbuf->block_count);
    if (rbuf_pos(rbuf->w_pos) == rbuf_pos(rbuf->r_pos)) {
        /* overflow */
        _rbuf_pos_increment(rbuf->r_pos, rbuf->block_count);
        ret = -2;
    }
    rbuf->ring_length = ringbuffer_get_length(rbuf);    
    return ret;
}

int 
ringbuffer_read(struct ringbuffer *rbuf, 
                    char *buffer, 
                    size_t length)
{
    int ret = 0;

    if (length > rbuf->block_length)
        return -1;

    rbuf->ring_length = ringbuffer_get_length(rbuf);
    if (rbuf->ring_length > 0) {
        memmove(buffer, rbuf->memory + rbuf->block_length*rbuf_pos(rbuf->r_pos), length);
        _rbuf_pos_increment(rbuf->r_pos, rbuf->block_count);
        rbuf->ring_length = ringbuffer_get_length(rbuf);
    } else
        ret = -2;
    return ret;
}

size_t
ringbuffer_get_length(struct ringbuffer *rbuf)
{
    size_t len = 0;

    if (rbuf_pos(rbuf->r_pos) <= rbuf_pos(rbuf->w_pos)) {
        len = rbuf_pos(rbuf->w_pos) - rbuf_pos(rbuf->r_pos);
    } else {
        len = rbuf->block_count - rbuf_pos(rbuf->r_pos) + rbuf_pos(rbuf->w_pos);
    }
    return len;
}
