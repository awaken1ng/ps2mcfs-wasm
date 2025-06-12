#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE   (512)

#ifdef __cplusplus
extern "C"{
#endif

void genblock(uint32_t card_size, size_t pos, void *vbuf);

#ifdef __cplusplus
}
#endif
