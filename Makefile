export PATH := /opt/toolchains/or1k-elf/bin:$(PATH)
CROSS_COMPILE ?= or1k-elf-

# Commands
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD =  $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip

# Compiler flags
CFLAGS = -O3 -fno-common -fno-builtin -ffreestanding -fno-exceptions -ffunction-sections -DDEBUG

# Linker flags
LDFLAGS = -static -nostartfiles -nodefaultlibs -Wl,--gc-sections -Wl,--require-defined=_start $(CFLAGS)

# Sources
SRC = main.c clk.c uart.c gpio.c debug.c timer.c libgcc.c
COBJ = $(SRC:.c=.o)

all: arisc-fw.code

boot/boot.code:
	make -C boot

arisc-fw.code: arisc-fw
	$(OBJCOPY) -O binary --reverse-bytes=4 $< $@

arisc-fw: $(COBJ) start.o
	$(CC) $(LDFLAGS) -Tarisc-fw.ld start.o $(COBJ) -lor1k -o $@
	$(STRIP) -x $@
	$(OBJDUMP) -D $@ > $@.as
	$(SIZE) $@

$(COBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

start.o: start.S boot/boot.code
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(COBJ) start.o arisc-fw arisc-fw.code arisc-fw.as
	make -C boot clean

