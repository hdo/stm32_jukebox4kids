include ../../../../Makefile.common

vpath %.c src

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -Iinc -I../../../CMSIS/Include -I../../../CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I../../../conf -I../../../USB_OTG/inc
CFLAGS += -I../../Core/inc

# Sources
SRCS = usbd_cdc_core.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libusbdevcdc.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	