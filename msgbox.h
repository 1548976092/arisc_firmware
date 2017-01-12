#ifndef __MSGBOX_H__
#define __MSGBOX_H__

#include <stdint.h>

void msgbox_init(void);
int msgbox_read(uint32_t queue, uint32_t* val);
int msgbox_write(uint32_t queue, uint32_t val);

#endif
