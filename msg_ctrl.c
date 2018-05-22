/*
    --- ARM-ARISC message control module -----------------------------------
*/

#include <string.h>
#include "msg_ctrl.h"




// private vars

static struct msg_t * msg_arisc[MSG_MAX_CNT] = {0};
static struct msg_t * msg_arm[MSG_MAX_CNT] = {0};

static struct msg_recv_callback_t msg_recv_callback[MSG_RECV_CALLBACK_CNT] = {0};
static uint8_t msg_recv_callback_max_id = 0;




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
    static uint8_t m;
    static uint8_t c;

    // walk through all arm messages
    for( m = 0; m < MSG_MAX_CNT; ++m )
    {
        // check for new arm messages
        if ( msg_arm[m]->unread && !msg_arm[m]->locked )
        {
            // walk through all `message received` callbacks
            for( c = 0; c <= msg_recv_callback_max_id; ++c )
            {
                // if received message must be sent to this callback
                if (
                    msg_recv_callback[c].used &&
                    msg_recv_callback[c].msg_type == msg_arm[m]->type
                ) {
                    // call function with message data as parameters
                    (*msg_recv_callback[c].func)
                    (
                        msg_arm[m]->type,
                        msg_arm[m]->msg,
                        msg_arm[m]->length
                    );
                }
            }

            // message read
            msg_arm[m]->unread = 0;
        }
    }
}




int8_t msg_send(uint8_t type, uint8_t * msg, uint8_t length)
{
    static uint8_t m;

    // walk through all arisc message slots
    for( m = 0; m < MSG_MAX_CNT; ++m )
    {
        // if this message slot is free
        if ( !msg_arisc[m]->unread )
        {
            // lock message slot while adding message data
            msg_arisc[m]->locked = 1;

            // set message data
            msg_arisc[m]->unread = 1;
            msg_arisc[m]->type   = type;
            msg_arisc[m]->length = length;
            memcpy(&msg_arisc[m]->msg, msg, length);

            // unlock message slot
            msg_arisc[m]->locked = 0;

            // return `message sent`
            return 0;
        }
    }

    // return `message not sent`
    return -1;
}




int8_t msg_add_recv_callback(uint8_t msg_type, int32_t (*func)(uint8_t, uint8_t*, uint8_t))
{
    static uint8_t c;

    // find free callback slot
    for( c = 0; c <= msg_recv_callback_max_id; ++c )
    {
        if ( !msg_recv_callback[c].used ) break;
    }

    if ( c == msg_recv_callback_max_id )
    {
        // return `callback wasn't added` if we have no free slots
        if ( (c+1) >= MSG_RECV_CALLBACK_CNT ) return -1;

        msg_recv_callback_max_id++;
        c = msg_recv_callback_max_id;
    }

    // add the callback to the list
    msg_recv_callback[c].used = 1;
    msg_recv_callback[c].msg_type = msg_type;
    msg_recv_callback[c].func = func;

    // return callback id
    return c;
}

int8_t msg_remove_recv_callback(uint8_t callback_id)
{
    if ( callback_id > msg_recv_callback_max_id ) return -1;
    if ( !msg_recv_callback[callback_id].used ) return -1;

    if ( callback_id && callback_id == msg_recv_callback_max_id )
    {
        msg_recv_callback_max_id--;
    }

    // remove callback from the list
    msg_recv_callback[callback_id].used = 0;

    // return `callback removed`
    return 0;
}
