include ../../Makefile.common

vpath %.c src

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -Iinc -I../CMSIS/Include -I../CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -I../conf

# Sources
SRCS = usb_core.c usb_dcd.c usb_dcd_int.c usb_hcd.c
SRCS += usb_hcd_int.c usb_otg.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libusbcore.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	