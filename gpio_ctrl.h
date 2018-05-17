/*
 * GPIO control module
 */

#ifndef _GPIO_CTRL_H
#define _GPIO_CTRL_H

#define GPIO_BASE       0X01C20800
#define GPIO_R_BASE     0x01f02c00
#define GPIO_BANK_SIZE  0x24

#define GPIO_CFG_INDEX(pin)     (((pin) & 0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin)    (((pin) & 0x7) << 2)

#define GPIO_PORTS_CNT  8 // max = 16

enum // port names
{
    PA, PB, PC, PD,
    PE, PF, PG, PL
};

enum // port pin names
{
    PA0 =0x0,       PA1 =0x1,           PA2 =0x2,           PA3 =0x4,
    PA4 =0x8,       PA5 =0x10,          PA6 =0x20,          PA7 =0x40,
    PA8 =0x80,      PA9 =0x100,         PA10=0x200,         PA11=0x400,
    PA12=0x800,     PA13=0x1000,        PA14=0x2000,        PA15=0x4000,
    PA16=0x8000,    PA17=0x10000,       PA18=0x20000,       PA19=0x40000,
    PA20=0x80000,   PA21=0x100000,      PA22=0x200000,      PA23=0x400000,
    PA24=0x800000,  PA25=0x1000000,     PA26=0x2000000,     PA27=0x4000000,
    PA28=0x8000000, PA29=0x10000000,    PA30=0x20000000,    PA31=0x40000000,

    PB0 =0x0,       PB1 =0x1,           PB2 =0x2,           PB3 =0x4,
    PB4 =0x8,       PB5 =0x10,          PB6 =0x20,          PB7 =0x40,
    PB8 =0x80,      PB9 =0x100,         PB10=0x200,         PB11=0x400,
    PB12=0x800,     PB13=0x1000,        PB14=0x2000,        PB15=0x4000,
    PB16=0x8000,    PB17=0x10000,       PB18=0x20000,       PB19=0x40000,
    PB20=0x80000,   PB21=0x100000,      PB22=0x200000,      PB23=0x400000,
    PB24=0x800000,  PB25=0x1000000,     PB26=0x2000000,     PB27=0x4000000,
    PB28=0x8000000, PB29=0x10000000,    PB30=0x20000000,    PB31=0x40000000,

    PC0 =0x0,       PC1 =0x1,           PC2 =0x2,           PC3 =0x4,
    PC4 =0x8,       PC5 =0x10,          PC6 =0x20,          PC7 =0x40,
    PC8 =0x80,      PC9 =0x100,         PC10=0x200,         PC11=0x400,
    PC12=0x800,     PC13=0x1000,        PC14=0x2000,        PC15=0x4000,
    PC16=0x8000,    PC17=0x10000,       PC18=0x20000,       PC19=0x40000,
    PC20=0x80000,   PC21=0x100000,      PC22=0x200000,      PC23=0x400000,
    PC24=0x800000,  PC25=0x1000000,     PC26=0x2000000,     PC27=0x4000000,
    PC28=0x8000000, PC29=0x10000000,    PC30=0x20000000,    PC31=0x40000000,

    PD0 =0x0,       PD1 =0x1,           PD2 =0x2,           PD3 =0x4,
    PD4 =0x8,       PD5 =0x10,          PD6 =0x20,          PD7 =0x40,
    PD8 =0x80,      PD9 =0x100,         PD10=0x200,         PD11=0x400,
    PD12=0x800,     PD13=0x1000,        PD14=0x2000,        PD15=0x4000,
    PD16=0x8000,    PD17=0x10000,       PD18=0x20000,       PD19=0x40000,
    PD20=0x80000,   PD21=0x100000,      PD22=0x200000,      PD23=0x400000,
    PD24=0x800000,  PD25=0x1000000,     PD26=0x2000000,     PD27=0x4000000,
    PD28=0x8000000, PD29=0x10000000,    PD30=0x20000000,    PD31=0x40000000,

    PE0 =0x0,       PE1 =0x1,           PE2 =0x2,           PE3 =0x4,
    PE4 =0x8,       PE5 =0x10,          PE6 =0x20,          PE7 =0x40,
    PE8 =0x80,      PE9 =0x100,         PE10=0x200,         PE11=0x400,
    PE12=0x800,     PE13=0x1000,        PE14=0x2000,        PE15=0x4000,
    PE16=0x8000,    PE17=0x10000,       PE18=0x20000,       PE19=0x40000,
    PE20=0x80000,   PE21=0x100000,      PE22=0x200000,      PE23=0x400000,
    PE24=0x800000,  PE25=0x1000000,     PE26=0x2000000,     PE27=0x4000000,
    PE28=0x8000000, PE29=0x10000000,    PE30=0x20000000,    PE31=0x40000000,

    PF0 =0x0,       PF1 =0x1,           PF2 =0x2,           PF3 =0x4,
    PF4 =0x8,       PF5 =0x10,          PF6 =0x20,          PF7 =0x40,
    PF8 =0x80,      PF9 =0x100,         PF10=0x200,         PF11=0x400,
    PF12=0x800,     PF13=0x1000,        PF14=0x2000,        PF15=0x4000,
    PF16=0x8000,    PF17=0x10000,       PF18=0x20000,       PF19=0x40000,
    PF20=0x80000,   PF21=0x100000,      PF22=0x200000,      PF23=0x400000,
    PF24=0x800000,  PF25=0x1000000,     PF26=0x2000000,     PF27=0x4000000,
    PF28=0x8000000, PF29=0x10000000,    PF30=0x20000000,    PF31=0x40000000,

    PG0 =0x0,       PG1 =0x1,           PG2 =0x2,           PG3 =0x4,
    PG4 =0x8,       PG5 =0x10,          PG6 =0x20,          PG7 =0x40,
    PG8 =0x80,      PG9 =0x100,         PG10=0x200,         PG11=0x400,
    PG12=0x800,     PG13=0x1000,        PG14=0x2000,        PG15=0x4000,
    PG16=0x8000,    PG17=0x10000,       PG18=0x20000,       PG19=0x40000,
    PG20=0x80000,   PG21=0x100000,      PG22=0x200000,      PG23=0x400000,
    PG24=0x800000,  PG25=0x1000000,     PG26=0x2000000,     PG27=0x4000000,
    PG28=0x8000000, PG29=0x10000000,    PG30=0x20000000,    PG31=0x40000000,

    PL0 =0x0,       PL1 =0x1,           PL2 =0x2,           PL3 =0x4,
    PL4 =0x8,       PL5 =0x10,          PL6 =0x20,          PL7 =0x40,
    PL8 =0x80,      PL9 =0x100,         PL10=0x200,         PL11=0x400,
    PL12=0x800,     PL13=0x1000,        PL14=0x2000,        PL15=0x4000,
    PL16=0x8000,    PL17=0x10000,       PL18=0x20000,       PL19=0x40000,
    PL20=0x80000,   PL21=0x100000,      PL22=0x200000,      PL23=0x400000,
    PL24=0x800000,  PL25=0x1000000,     PL26=0x2000000,     PL27=0x4000000,
    PL28=0x8000000, PL29=0x10000000,    PL30=0x20000000,    PL31=0x40000000
};

// port bank names
#define GPIO_BANK_A     0
#define GPIO_BANK_B     1
#define GPIO_BANK_C     2
#define GPIO_BANK_D     3
#define GPIO_BANK_E     4
#define GPIO_BANK_F     5
#define GPIO_BANK_G     6
#define GPIO_BANK_L     7

// pin function
#define GPIO_FUNC_INPUT         0
#define GPIO_FUNC_OUTPUT        1
#define GPIO_FUNC_BANK_A_UART0  2
#define GPIO_FUNC_BANK_A_I2C0   2
#define GPIO_FUNC_BANK_A_I2C1   3
#define GPIO_FUNC_BANK_E_I2C2   3
#define GPIO_FUNC_BANK_L_I2C3   2




// export public methods
void gpio_ctrl_init(void);
void gpio_ctrl_base_thread();




#endif
