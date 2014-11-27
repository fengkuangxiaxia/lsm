obj-m += lsm.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f *.mod.c
	rm -f *.mod.o
	rm -f *.o
	rm -f *.ko
	rm -f module*
	rm -f Module.*
	rm -f *.c~
