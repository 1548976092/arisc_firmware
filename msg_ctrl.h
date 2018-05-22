/*
    --- ARM-ARISC message control module -----------------------------------
*/

#ifndef _MSG_CTRL_H
#define _MSG_CTRL_H

#include <stdint.h>




#define SRAM_A2_SIZE            (48*1024)
#define SRAM_A2_ADDR            0x00000000 // for ARM use 0x00040000
#define ARISC_CONF_SIZE         2048
#define ARISC_CONF_ADDR         (SRAM_A2_ADDR + SRAM_A2_SIZE - ARISC_CONF_SIZE)

#define MSG_BLOCK_SIZE          2048
#define MSG_BLOCK_ADDR          (ARISC_CONF_ADDR - MSG_BLOCK_SIZE)

#define MSG_CPU_BLOCK_SIZE      1024
#define MSG_ARISC_BLOCK_ADDR    (MSG_BLOCK_ADDR + 0)
#define MSG_ARM_BLOCK_ADDR      (MSG_BLOCK_ADDR + MSG_CPU_BLOCK_SIZE)

#define MSG_MAX_CNT             4
#define MSG_MAX_LEN             (MSG_CPU_BLOCK_SIZE / MSG_MAX_CNT)

#define MSG_RECV_CALLBACK_CNT   64




struct msg_t
{
    uint8_t unread;
    uint8_t locked;
    uint8_t type;
    uint8_t length;
    uint8_t msg[MSG_MAX_LEN - 4];
};

struct msg_recv_callback_t
{
    uint8_t used;
    uint8_t msg_type;
    int32_t (*func)(uint8_t, uint8_t*, uint8_t);
};




// export public methods

void msg_ctrl_init(void);
void msg_ctrl_base_thread(void);

int8_t msg_send(uint8_t type, uint8_t * msg, uint8_t length);

int8_t msg_add_recv_callback(uint8_t msg_type, int32_t (*func)(uint8_t, uint8_t*, uint8_t));
int8_t msg_remove_recv_callback(uint8_t callback_id);




#endif
