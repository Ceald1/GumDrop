obj-m += gumdrop.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod gumdrop.ko

uninstall:
	sudo rmmod gumdrop

reload: uninstall install

dmesg:
	dmesg | tail -20

.PHONY: all clean install uninstall reload dmesg
