obj-m := message_slot.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
 
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

compile:
    gcc -O3 -Wall -std=c11 message_sender.c
    gcc -O3 -Wall -std=c11 message_reader.c

compile_reader:
    gcc -O3 -Wall -std=c11 message_reader.c

compile_sender:
    gcc -O3 -Wall -std=c11 message_sender.c