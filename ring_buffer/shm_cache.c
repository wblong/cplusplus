#include <stdio.h>
#include <windows.h>

#include "shm_cache.h"
#include "ringbuffer.h"

#define strcpy(d, s)  strcpy_s(d, strlen(s)+1, s)

struct shm_cache {
    char name[256];
    char event_name[256];
    size_t block_length;
    size_t count;
    int create;

    void *fd;
    HANDLE event;
    char *memory;
    struct ringbuffer *rbuf;    
};

#define read_pos_address(sc)  (sc->memory+sc->block_length*(sc->count+1))
#define write_pos_address(sc) (read_pos_address(sc)+sizeof(int))

static int
_shm_cache_initialize(struct shm_cache *shm_cah)
{
    int ret = 0;
    size_t length = 0;

    if (shm_cah->create) {
        /* 8*2 space is ringbuffer' read pos and write pos. */
        length = shm_cah->block_length*(shm_cah->count+1)+2*sizeof(int);
		//!INVALID_HANDLE_VALUE 创建一个物理文件无关的内存映射
		//!创建一个名为shm_cah->name的长度为length字节的有名映射文件
        shm_cah->fd = CreateFileMapping(INVALID_HANDLE_VALUE, 
                                        NULL, 
                                        PAGE_READWRITE | SEC_COMMIT, 
                                        0, 
                                        (DWORD) length, 
                                        shm_cah->name);
		//!Event系统内核对象
        shm_cah->event = CreateEvent(NULL, FALSE, TRUE, shm_cah->event_name);
    } else {
        shm_cah->fd = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, shm_cah->name);
        shm_cah->event = OpenEvent(EVENT_ALL_ACCESS, FALSE, shm_cah->event_name);
    }

    if (shm_cah->fd && shm_cah->event) {
        shm_cah->memory = MapViewOfFile(shm_cah->fd, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        if (NULL == shm_cah->memory) {
            printf("error>%s, line %d>MapViewOfFile() err=%d\n", __FUNCTION__, __LINE__, GetLastError());
            goto error;
        }

        if (shm_cah->create)
            memset(shm_cah->memory, 0x00, length);

        shm_cah->rbuf = ringbuffer_create(shm_cah->memory, 
                                        shm_cah->block_length,
                                        shm_cah->count,
                                        (int *) read_pos_address(shm_cah),
                                        (int *) write_pos_address(shm_cah));
        if (!shm_cah->rbuf)
            goto error;
    } else {
        goto error;
    }
    return ret;
error:
    return -1;
}

static void
_shm_cache_finalize(struct shm_cache **sc)
{
    struct shm_cache *shm_cah = sc ? *sc : NULL;

    if (shm_cah) {
        if (shm_cah->event) 
            CloseHandle(shm_cah->event);
        if (shm_cah->fd)
            UnmapViewOfFile(shm_cah->memory);
        if (shm_cah->rbuf)
            ringbuffer_delete(shm_cah->rbuf);
        free(shm_cah);
        *sc = NULL;
    }
}

static struct shm_cache * 
_shm_cache_allocate(const char *name, 
                            size_t block_length, 
                            size_t count, 
                            int create)
{
    struct shm_cache *shm_cah = (struct shm_cache *) calloc(1, sizeof(*shm_cah));

    if (shm_cah) {
        size_t name_len = strlen(name);
        strcpy(shm_cah->name, name);
        strcpy(shm_cah->event_name, name);
        strcpy(&(shm_cah->event_name[name_len]), "_event");
        shm_cah->block_length = block_length;
        shm_cah->count = count;
        shm_cah->create = create;
        if (_shm_cache_initialize(shm_cah))
            _shm_cache_finalize(&shm_cah);
    }

    return shm_cah;
}

struct shm_cache *
shm_cache_create(const char *name, 
                        size_t block_length, 
                        size_t count)
{
    return _shm_cache_allocate(name, block_length, count, 1);
}

void 
shm_cache_delete(struct shm_cache *shm_cah)
{
    _shm_cache_finalize(&shm_cah);
}

struct shm_cache *
shm_cache_open(const char *name, 
                    size_t block_length, 
                    size_t count)
{
    return _shm_cache_allocate(name, block_length, count, 0);
}

void 
shm_cache_close(struct shm_cache *shm_cah)
{
    _shm_cache_finalize(&shm_cah);
}

int 
shm_cache_write(struct shm_cache *shm_cah, 
                const char *buffer, 
                size_t length,
                size_t timeout_ms)
{
    int ret = SHM_CAH_OK;

    if (WaitForSingleObject(shm_cah->event, (DWORD) timeout_ms) == WAIT_OBJECT_0) {
        ret = ringbuffer_write(shm_cah->rbuf, buffer, length);
        SetEvent(shm_cah->event);
        if (ret == -1)
            ret = SHM_CAH_ERROR_PARAMETER_INVALID;
        else if (ret == -2)
            ret = SHM_CAH_ERROR_OVERFLOW;
    } else
        ret = SHM_CAH_ERROR_TIMEOUT;
    return ret;
}

int 
shm_cache_read(struct shm_cache *shm_cah, 
                char *buffer, 
                size_t length,
                size_t timeout_ms)
{
    int ret = SHM_CAH_OK;

    if (WaitForSingleObject(shm_cah->event, (DWORD) timeout_ms) == WAIT_OBJECT_0) {
        ret = ringbuffer_read(shm_cah->rbuf, buffer, length);
        SetEvent(shm_cah->event);
        if (ret == -1)
            ret = SHM_CAH_ERROR_PARAMETER_INVALID;
        else if (ret == -2)
            ret = SHM_CAH_ERROR_NO_DATA;
    } else
        ret = SHM_CAH_ERROR_TIMEOUT;
    return ret;
}
