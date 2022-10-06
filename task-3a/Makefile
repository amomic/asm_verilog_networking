KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: firewall

firewall: *.cpp
	g++ -std=c++17 -Wall -Werror -g framework/*.cpp *.cpp -o firewall -lnfnetlink -lnetfilter_queue

kmod.ko: framework/kmod.c
	$(MAKE) -C $(KERNELDIR) M=$(PWD)/framework modules
	mv framework/kmod.ko .
	rm -rf framework/*.o framework/.*.cmd framework/*.mod* framework/modules.order framework/Module.symvers

clean:
	rm -rf firewall *.o *.ko

install: kmod.ko
	sudo insmod ./kmod.ko

uninstall:
	sudo rmmod kmod

run: firewall
	sudo ./firewall

valgrind: firewall
	sudo valgrind \
	  --leak-check=full --show-leak-kinds=all --show-error-list=yes \
	  --suppressions=framework/valgrind_suppression \
	  ./firewall

.PHONY: install, uninstall, run, valgrind
