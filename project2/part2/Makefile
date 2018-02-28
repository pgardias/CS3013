obj-m := pt2.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	gcc pt2test.c -o test
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f test

