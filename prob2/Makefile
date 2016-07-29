obj-m += fibonacci.o
fibonacci-objs := fibonacci_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules \
  && $(CC) fibonacci_client.c -o fibonacci_client

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean \
  && rm fibonacci_client
