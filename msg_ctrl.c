/*
    --- ARM-ARISC message control module -----------------------------------
*/

#include <string.h>
#include "msg_ctrl.h"




// private vars

static struct msg_t * msg_arisc[MSG_MAX_CNT] = {0};
static struct msg_t * msg_arm[MSG_MAX_CNT] = {0};




// public methods

void msg_ctrl_init(void)
{
    uint8_t m = 0;

    // messages memory block cleanup
    memset((uint8_t*)MSG_BLOCK_ADDR, 0, MSG_BLOCK_SIZE);

    // assign messages pointers
    for ( ; m < MSG_MAX_CNT; ++m )
    {
        msg_arisc[m] = (struct msg_t *) (MSG_ARISC_BLOCK_ADDR + m * MSG_MAX_LEN);
        msg_arm[m]   = (struct msg_t *) (MSG_ARM_BLOCK_ADDR   + m * MSG_MAX_LEN);
    }
}

void msg_ctrl_base_thread(void)
{
    // msg id
    static uint8_t m;

    // walk through all arm messages
    for( m = 0; m < MSG_MAX_CNT; ++m )
    {
        // check for new arm messages
        if ( msg_arm[m]->unread && !msg_arm[m]->locked )
        {
            // TODO - call message received callback

            // message read
            msg_arm[m]->unread = 0;
        }
    }
}
