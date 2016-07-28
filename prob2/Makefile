obj-m += fibonacci.o
fibonacci-objs := prob2-1.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules \
  && $(CC) prob2-2.c -o tester

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean \
  && rm tester
