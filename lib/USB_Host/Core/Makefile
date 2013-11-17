include ../../../Makefile.common

vpath %.c src

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -Iinc -I../../CMSIS/Include -I../../CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I../../conf -I../../USB_OTG/inc

# Sources
SRCS = usbh_core.c usbh_hcs.c usbh_ioreq.c usbh_stdreq.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libusbhostcore.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	