/**
 * @file    mod_msg.c
 *
 * @brief   ARM-ARISC message control module
 *
 * This module implements an API to communication
 * between ARISC and ARM processors
 */

#include <string.h>
#include "mod_msg.h"




// private vars

static struct msg_t * msg_arisc[MSG_MAX_CNT] = {0};
static struct msg_t * msg_arm[MSG_MAX_CNT] = {0};

static struct msg_recv_callback_t msg_recv_callback[MSG_RECV_CALLBACK_CNT] = {0};
static uint8_t msg_recv_callback_max_id = 0;




// public methods

/**
 * @brief   module init
 * @note    call this function only once before msg_module_base_thread()
 * @retval  none
 */
void msg_module_init(void)
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

/**
 * @brief   module base thread
 * @note    call this function at the top of main loop
 * @retval  none
 */
void msg_module_base_thread(void)
{
    static uint8_t m;
    static uint8_t c;

    // walk through all arm messages
    // TODO - this cycle can cause a problem with messages reading order
    // TODO - change this type of cycle to FIFO
    for( m = 0; m < MSG_MAX_CNT; ++m )
    {
        // check for new arm messages
        if ( !msg_arm[m]->unread || msg_arm[m]->locked ) continue;

        // walk through all `message received` callbacks
        for( c = msg_recv_callback_max_id; c--; )
        {
            // check callback's message type
            if ( !msg_recv_callback[c].used || msg_recv_callback[c].msg_type != msg_arm[m]->type ) continue;

            // call function with message data as parameters
            (*msg_recv_callback[c].func)(msg_arm[m]->type, msg_arm[m]->msg, msg_arm[m]->length);
        }

        // message read
        msg_arm[m]->unread = 0;
    }
}




/**
 * @brief   send a message to the ARM cpu
 *
 * @param   type    user defined message type (0..0xFF)
 * @param   msg     pointer to the message buffer
 * @param   length  the length of a message ( 0 .. (MSG_MAX_LEN-4) )
 *
 * @retval   0 (message sent)
 * @retval  -1 (message not sent)
 */
int8_t msg_send(uint8_t type, uint8_t * msg, uint8_t length)
{
    static uint8_t m;

    // walk through all arisc message slots
    // TODO - this cycle can cause a problem with messages reading order
    // TODO - change this type of cycle to FIFO
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

            // copy message to the buffer
            memset( (uint8_t*) ((uint8_t*)&msg_arisc[m]->msg + length/4*4), 0, 4 );
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




/**
 * @brief   add the function to the list of "message received callbacks"
 *
 * @note    the callback function must have 3 arguments,
 *          same as for the msg_send() function
 *
 * @param   msg_type    user defined message type (0..0xFF)
 * @param   func        pointer to the callback function
 *
 * @retval  0..MSG_RECV_CALLBACK_CNT (callback id)
 * @retval  -1 (callback wasn't added)
 */
int8_t msg_recv_callback_add(uint8_t msg_type, msg_recv_func_t func)
{
    static uint8_t c;

    // find free callback slot
    for( c = 0; c < MSG_RECV_CALLBACK_CNT; ++c )
    {
        if ( msg_recv_callback[c].used ) continue;
    }

    // return if there are no free callback slots
    if ( c >= MSG_RECV_CALLBACK_CNT ) return -1;

    // if needed increase callback max ID
    if ( c > msg_recv_callback_max_id ) msg_recv_callback_max_id = c;

    // add the callback to the list
    msg_recv_callback[c].used = 1;
    msg_recv_callback[c].msg_type = msg_type;
    msg_recv_callback[c].func = func;

    // return callback id
    return c;
}

/**
 * @brief   remove the callback function from the list of "message received callbacks"
 *
 * @param   callback_id     callback function id (0..MSG_RECV_CALLBACK_CNT)
 *                          returned by the msg_add_recv_callback()
 *
 * @retval   0 (callback removed)
 * @retval  -1 (callback wasn't removed)
 */
int8_t msg_recv_callback_remove(uint8_t callback_id)
{
    static int8_t c;

    if ( callback_id > msg_recv_callback_max_id ) return -1;
    if ( !msg_recv_callback[callback_id].used ) return -1;

    // remove callback from the list
    msg_recv_callback[callback_id].used = 0;

    // if needed decrease callback max ID
    if ( callback_id == msg_recv_callback_max_id )
    {
        for( c = callback_id; c--; )
        {
            if ( !msg_recv_callback[c].used ) continue;
        }

        msg_recv_callback_max_id = c < 0 ? 0 : c;
    }

    // return `callback removed`
    return 0;
}




/**
    @example mod_msg.c

    <b>Usage example 1</b>: sending mirror messages back to the ARM cpu
                            with message count limit of 100

    @code
        #include <stdint.h>
        #include "mod_msg.h"

        int callback_id = 0; // messages callback id
        int msg_counter = 0; // messages counter

        // callback for the `message received` event
        int32_t volatile msg_received(uint8_t type, uint8_t * msg, uint8_t length)
        {
            // send mirror message
            msg_send(type, msg, length);
            // increase messages count
            msg_counter++;
            // abort messages receiving after 100 incoming messages
            if ( msg_counter >= 100 ) msg_recv_callback_remove(callback_id);
            // normal exit
            return 0;
        }

        int main(void)
        {
            // module init
            msg_module_init();

            // assign incoming messages callback for the message type 123
            callback_id = msg_recv_callback_add(123, (msg_recv_func_t) &msg_received);

            // main loop
            for(;;)
            {
                // real reading/sending of a messages
                msg_module_base_thread();
            }

            return 0;
        }
    @endcode
*/
