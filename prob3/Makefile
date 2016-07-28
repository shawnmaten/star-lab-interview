obj-m += simple_db.o
simple_db-objs := simple_db_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules \
  && $(CC) simple_db_client.c -o simple_db_client

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean \
  && rm -r simple_db_client modules.order *.o *.ko *.mod* .*.cmd
