include ../Makefile.common

# Build all libraries

all:
	$(MAKE) -C STM32F4xx_StdPeriph_Driver FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C USB_OTG FLOAT_TYPE=$(FLOAT_TYPE)
#	$(MAKE) -C USB_Device/Core FLOAT_TYPE=$(FLOAT_TYPE)
#	$(MAKE) -C USB_Device/Class/cdc FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C USB_Host/Core FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C USB_Host/Class/MSC FLOAT_TYPE=$(FLOAT_TYPE)
	$(MAKE) -C fat_fs FLOAT_TYPE=$(FLOAT_TYPE)
#	$(MAKE) -C fatfs FLOAT_TYPE=$(FLOAT_TYPE)

clean:
	$(MAKE) clean -C STM32F4xx_StdPeriph_Driver
	$(MAKE) clean -C USB_OTG
#	$(MAKE) clean -C USB_Device/Core
#	$(MAKE) clean -C USB_Device/Class/cdc
	$(MAKE) clean -C USB_Host/Core
	$(MAKE) clean -C USB_Host/Class/MSC
	$(MAKE) clean -C fat_fs
#	$(MAKE) clean -C fatfs
