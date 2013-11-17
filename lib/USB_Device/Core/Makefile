include ../../../Makefile.common

vpath %.c src

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -Iinc -I../../CMSIS/Include -I../../CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I../../conf -I../../USB_OTG/inc

# Sources
SRCS = usbd_core.c usbd_ioreq.c usbd_req.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libusbdevcore.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	