obj-m += hid-tstp-mtouch.o
INSTALL_DIR ?= /lib/modules/$(shell uname -r)
KDIR ?= $(INSTALL_DIR)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	cp hid-tstp-mtouch.ko "$(INSTALL_DIR)/"
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
install:
	modprobe hid-tstp-mtouch
