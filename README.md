# Star Lab Coding Challenge

Shawn Aten (shawnmaten@gmail.com)

## Problem 1
This program looks for a system's cpu model, mac address, board uuid, and 
Docker id and creates a SHA-256 hash from the available values. This hash can 
be used as a unique fingerprint. For a bare-metal system it will remain the 
same unless hardware is changed. It will change when a virtual machine is 
created from a snapshot or cloned and it will change with every new run of a 
Docker image. To use simply run `make` then `./fingerprint`. You will need an 
OpenSSl dev library installed to make such as libssl-dev on Ubuntu.

```
./fingerprint 
Attribute #0 CPU Model:
  Intel(R) Xeon(R) CPU E5-2650L v3 @ 1.80GHz
Attribute #1 MAC:
  04:01:36:c6:b2:01
Attribute #2 Board UUID:
  5FD7C19A-9DA7-4BB2-95C6-2E43DB44D2CE

System Fingerprint:
  2d05053c595ae4ca3185bc0845460fd515e9cd1e28a958cb8299b69864896073
```

## Problem 2
There are two programs for this problem. The first, defined in
fibonacci _ module.c, creates a linux kernel module for a character device. The 
second, defined in fibonacci _ client.c is a simple tester for the device. Run 
`make` then `sudo insmod fibonacci.ko` to install the device. 
/dev/fibonacci should be present on the system. If you run `sudo cat
/dev/fibonacci` it will return 0; You can view the kernel module output in 
/var/log/kern.log (or wherever it is on your system). Look for output with 
"Fibonacci:". Use the tester as below And finally remove the module with `sudo 
rmmod fibonacci`.

```
./fibonacci_client 13
The 0 to 13th fibonacci nums as returned /dev/fibonacci are:
0 1 1 2 3 5 8 13 21 34 55 89 144
```

## Problem 3
There's a kernel module, simple _ db _ module.c, and client tester,
simple _ db _ client.c, for this solution. They send fixed-length messages to
each other using netlink sockets. The kernel module stores key:value "blobs" in
memory that it allocates as needed. To compile use `make` then `sudo insmod
simple_db.ko` to insert the module. Run the client as below. Press ctrl-d to
finish.

Terminal 1
```
./simple_db_client 
Do you want to SET or GET a variable?
set
Enter key:
animal
Enter value:
giraffe
Do you want to SET or GET a variable?
```

Terminal 2
```
./simple_db_client 
Do you want to SET or GET a variable?
get
Enter key:
animal 
Waiting for value...
Value for key animal:
giraffe
Do you want to SET or GET a variable?
```

