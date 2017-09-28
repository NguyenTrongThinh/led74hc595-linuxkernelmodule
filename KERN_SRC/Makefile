
obj-m:=led74hc595.o


KERN_SRC :=/lib/modules/$(shell uname -r)/build/
PWD:=$(shell pwd)

modules:
	make -C $(KERN_SRC) M=$(PWD) modules
	gcc ../testled74hc595.c -o ../app -g
	
install:
	make -C $(KERN_SRC) M=$(PWD) modules_install
	depmod -a

clean:
	make -C $(KERN_SRC) M=$(PWD) clean
	rm -f ../app