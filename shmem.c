#include "shmem.h"




// global shared memory
volatile struct global_shmem_t *shm =
    (struct global_shmem_t *) ARISC_SHMEM_ADDR;




// clean up shared memory
void shmem_clear_all()
{
    for ( int16_t b = sizeof shm; b--; )
    {
        *( (volatile uint8_t *)shm + b ) = 0;
    }
}
