obj-m := kmod.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

build:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
