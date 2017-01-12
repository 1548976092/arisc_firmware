export PATH := ../../../Toolchains/arm/bin:$(PATH)
CROSS_COMPILE ?= arm-linux-gnueabihf-

# Commands
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

CFLAGS = -Os -marm -march=armv7-a -DARM_BOOT -ffreestanding
LDFLAGS = -static -Wl,--gc-sections -nostartfiles -nostdlib

all: boot.code

boot.code: boot
	$(OBJCOPY) -O binary --reverse-bytes=4 $< $@

boot: main.c start.S msgbox.c boot.ld
	$(CC) $(CFLAGS) -c start.S -o start.o
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) $(CFLAGS) -c msgbox.c -o msgbox.o
	$(LD) $(LDFLAGS) -Tboot.ld start.o main.o msgbox.o -o $@

clean:
	rm -f boot.code boot start.o main.o msgbox.o

.PHONY: all clean
