#ifndef __SHM_CACHE__H_
#define __SHM_CACHE__H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * provide exchange data between two processs, base on windows share memory.
 */

enum {
    SHM_CAH_OK = 0,
    SHM_CAH_ERROR_PARAMETER_INVALID,
    SHM_CAH_ERROR_TIMEOUT,
    SHM_CAH_ERROR_OVERFLOW,
    SHM_CAH_ERROR_NO_DATA,
};

#define SHM_CACHE_INFINITE  0xFFFFFFFF

struct shm_cache;

/**
 * writer call
 */
struct shm_cache *
shm_cache_create(const char *name, 
                    size_t block_length, 
                    size_t count);

void 
shm_cache_delete(struct shm_cache *shm_cah);

/**
 * reader call
 */
struct shm_cache *
shm_cache_open(const char *name, 
                    size_t block_length, 
                    size_t count);

void 
shm_cache_close(struct shm_cache *shm_cah);

/**
 * Write.
 *
 * @param shm_cah
 * @param buffer
 * @param length
 * @param timeout_ms    millisecons, 0 on immediate, SHM_CACHE_INFINITE indicate INFINITE
 * @return SHM_CAH_OK                       on success
 *         SHM_CAH_ERROR_PARAMETER_INVALID  on parameter invalid
 *         SHM_CAH_ERROR_TIMEOUT            on timeout
 *         SHM_CAH_ERROR_OVERFLOW           on overflow
 */
int 
shm_cache_write(struct shm_cache *shm_cah, 
                const char *buffer, 
                size_t length,
                size_t timeout_ms);

/**
 * Read.
 *
 * @param shm_cah
 * @param buffer
 * @param length
 * @param timeout_ms    millisecons, 0 on immediate, SHM_CACHE_INFINITE indicate INFINITE
 * @return SHM_CAH_OK                       on success
 *         SHM_CAH_ERROR_PARAMETER_INVALID  on parameter invalid
 *         SHM_CAH_ERROR_TIMEOUT            on timeout
 *         SHM_CAH_ERROR_NO_DATA            on cache is empty
 */
int 
shm_cache_read(struct shm_cache *shm_cah, 
                char *buffer, 
                size_t length,
                size_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif