#include "shmem.h"




// global shared memory
volatile struct global_shmem_t *shm =
    (struct global_shmem_t *) ARISC_SHMEM_ADDR;
