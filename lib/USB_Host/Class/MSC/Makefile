include ../../../../Makefile.common

vpath %.c src

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -Iinc -I../../../CMSIS/Include -I../../../CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I../../../conf -I../../../USB_OTG/inc
CFLAGS += -I../../Core/inc
CFLAGS += -I../../../fat_fs/inc

# Sources
SRCS = usbh_msc_bot.c usbh_msc_core.c usbh_msc_fatfs.c usbh_msc_scsi.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libusbhostmsc.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	