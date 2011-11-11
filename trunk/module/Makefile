MODULENAME = ni4050
DEFINES   +=  -DMODULE -D__KERNEL__

#DEFINES += ni4050_DEBUG

TARGET = ni4050
ifneq ($(KERNELRELEASE),)

EXTRA_CFLAGS = $(DEFINES)
obj-m += $(MODULENAME).o

else   # We were called from command line

KERNEL_VERSION = `uname -r`
KERNELDIR := /lib/modules/$(KERNEL_VERSION)/build
PWD  := $(shell pwd)
MODULE_INSTALLDIR = /lib/modules/$(KERNEL_VERSION)/kernel/drivers/char/pcmcia

default:
	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) CC=$(CC) modules

install:
	mkdir -p $(MODULE_INSTALLDIR)
	rm -f $(MODULE_INSTALLDIR)/$(MODULENAME).ko
	install -c -m 0644 $(MODULENAME).ko $(MODULE_INSTALLDIR)
	/sbin/depmod -ae
	sudo pccardctl insert
	sleep 1
	sudo chmod a+rw /dev/nidmm0

uninstall:
	rm -f $(MODULE_INSTALLDIR)/$(MODULENAME).ko
	/sbin/depmod -aq
		
unload:
	sudo pccardctl eject && rmmod $(MODULENAME) 

endif

clean:
	rm -r -f *.o \
	*.o.cmd .$(MODULENAME).o.cmd  *.o *.ko *.mod.* .[a-z]* core *.i \
	*.symvers *.err
