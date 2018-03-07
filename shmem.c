#include "shmem.h"




// clean up shared memory
void shmem_clear_all()
{
    uint8_t *addr = (uint8_t *) SHM_ADDR;
    int b = SHM_SIZE;

    for ( ; b--; ) *addr++ = (uint8_t) 0;
}
