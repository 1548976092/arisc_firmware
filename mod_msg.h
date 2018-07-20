/**
 * @file    mod_msg.h
 *
 * @brief   ARM-ARISC message control module header
 *
 * This module implements an API to communication
 * between ARISC and ARM processors
 */

#ifndef _MOD_MSG_H
#define _MOD_MSG_H

#include <stdint.h>




#define SRAM_A2_SIZE            (48*1024)
#define SRAM_A2_ADDR            0x00000000 ///< for ARM use 0x00040000
#define ARISC_CONF_SIZE         2048
#define ARISC_CONF_ADDR         (SRAM_A2_ADDR + SRAM_A2_SIZE - ARISC_CONF_SIZE)

#define MSG_BLOCK_SIZE          4096
#define MSG_BLOCK_ADDR          (ARISC_CONF_ADDR - MSG_BLOCK_SIZE)

#define MSG_CPU_BLOCK_SIZE      2048
#define MSG_ARISC_BLOCK_ADDR    (MSG_BLOCK_ADDR + 0)
#define MSG_ARM_BLOCK_ADDR      (MSG_BLOCK_ADDR + MSG_CPU_BLOCK_SIZE)

#define MSG_MAX_CNT             32
#define MSG_MAX_LEN             (MSG_CPU_BLOCK_SIZE / MSG_MAX_CNT)
#define MSG_LEN                 (MSG_MAX_LEN - 4)

#define MSG_RECV_CALLBACK_CNT   32




#pragma pack(push, 1)
struct msg_t
{
    uint8_t unread;
    uint8_t locked; // actually not used at this moment
    uint8_t type;
    uint8_t length;
    uint8_t msg[MSG_LEN];
};
#pragma pack(pop)

typedef int32_t (*msg_recv_func_t)(uint8_t, uint8_t*, uint8_t);

struct msg_recv_callback_t
{
    uint8_t used;
    uint8_t msg_type;
    msg_recv_func_t func;
};




// export public methods

void msg_module_init(void);
void msg_module_base_thread(void);

int8_t msg_send(uint8_t type, uint8_t * msg, uint8_t length);

int8_t msg_recv_callback_add(uint8_t msg_type, msg_recv_func_t func);
int8_t msg_recv_callback_remove(uint8_t callback_id);




#endif
