include ../../Makefile.common

vpath %.c src
vpath %.c src/option

CFLAGS += -ffreestanding -nostdlib

# Includes
CFLAGS += -I../CMSIS/Include -I../CMSIS/Device/ST/STM32F4xx/Include
CFLAGS += -Iinc -I../conf

# Sources
SRCS = ff.c fattime.c ccsbcs.c

OBJS = $(SRCS:.c=.o)
LIBNAME = libfatfs.a

all: $(LIBNAME)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(LIBNAME): $(OBJS)
	$(AR) -r $@ $(OBJS)

clean:
	rm -f $(OBJS) $(LIBNAME)
	