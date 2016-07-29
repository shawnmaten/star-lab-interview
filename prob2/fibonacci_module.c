#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#define  DEVICE_NAME "fibonacci"
#define  CLASS_NAME  "char"

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
int num_digits(long long);
long long fibonacci(long long);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shawn Aten");
MODULE_DESCRIPTION("A simple linux char driver that returns fibonacci nums.");

static struct file_operations fops =
{
  .open = dev_open,
  .read = dev_read,
  .release = dev_release
};

static int major;
static int numOpen = 0;
static int bytes_left = -1;
static int bytes_read = 0;

static char *message = NULL;

static loff_t last_off = 0;

static struct class *class = NULL;

static struct device *device = NULL;

static int __init fibonacci_init(void) 
{
  printk(KERN_INFO "Fibonacci: Initializing\n");
 
  // Allocate major number
  major = register_chrdev(0, DEVICE_NAME, &fops);
  if ( major < 0 ) 
  {
    printk(KERN_ALERT "Fibonacci: Failed to register major number\n");
    return major;
  }
  printk(KERN_INFO "Fibonacci: Registered major number %d\n", major);
 
  // Register device class, cleanup if fails
  class = class_create(THIS_MODULE, CLASS_NAME);
  if ( IS_ERR(class) ) 
  {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_ALERT "Fibonacci: Failed to register device class\n");
    return PTR_ERR( class );
   }
  printk(KERN_INFO "Fibonacci: Device class registered correctly\n");
 
  // Register device driver, cleanup if fails
  device = device_create(class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
  if ( IS_ERR(device) ) 
  {
    class_destroy(class);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_ALERT "Fibonacci: Failed to create the device\n");
    return PTR_ERR(device);
  }
  printk(KERN_INFO "Fibonacci: Device class created correctly\n");

  return 0;
}

static void __exit fibonacci_exit(void)
{
  device_destroy(class, MKDEV(major, 0));
  class_unregister(class);
  class_destroy(class);
  unregister_chrdev(major, DEVICE_NAME);
  if ( message )
    kfree(message);
  printk(KERN_INFO "Fibonacci: Exited\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
  numOpen++;
  printk(KERN_INFO "Fibonacci: Device opened %d times\n", numOpen);
  return 0;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
  bytes_left = -1;
  printk(KERN_INFO "Fibonacci: Device closed\n");
  return 0;
}

static ssize_t dev_read(struct file *fp, char *buf, size_t len, loff_t *off) 
{
  int bytes_to_copy;
  
  long long fib_num;

  fib_num = 0;

  if ( last_off != *off )
    bytes_left = -1;
  last_off = *off;
  
  if ( bytes_left == -1 ) // Generate new fib num and send
  {
    if ( *off < 0 )
      return -EFAULT;

    fib_num = fibonacci(*off);
    
    bytes_left = num_digits(fib_num);
    bytes_read = 0;
    if ( message )
      kfree(message);
    message = kmalloc((bytes_left + 1) * sizeof(char), GFP_KERNEL);
    if ( message == NULL )
    {
      printk(KERN_INFO "Fibonacci: Faild to alloc %d bytes\n", bytes_left + 1);
      return -EFAULT;
    }
    sprintf(message, "%lld", fib_num);
  } 
  
  if ( bytes_left == 0 )
    bytes_to_copy = 0;
  else if ( bytes_left <= len )
    bytes_to_copy = bytes_left;
  else if ( bytes_left > len )
    bytes_to_copy = len;
  
  printk(KERN_INFO "Fibonacci: fib == %lld left == %d - offset == %lld\
    - to_copy == %d\n", fib_num, bytes_left, *off, bytes_to_copy);

  if ( bytes_to_copy )
  {
    int error_count;
    
    error_count = copy_to_user(buf, message+bytes_read, bytes_to_copy);
    if ( error_count != 0 )
    {
      bytes_left = -1;
      printk(KERN_INFO "Fibonacci: Faild to send %d bytes\n", error_count);
      return -EFAULT;
    }
    else
    {
      bytes_read += bytes_to_copy;
      bytes_left -= bytes_to_copy;
    }
  }

  if ( bytes_to_copy == 0 )
    bytes_left = -1;

  return bytes_to_copy;
}

int num_digits(long long num)
{
  
  int digits = 0;
  
  if ( num <= 0 )
    digits = 1;
  while ( num ) {
    num /= 10;
    digits++;
  }

  return digits;
}

long long fibonacci(long long k)
{
  long long i;
  long long nums[k+1];

  if ( k == 0 )
    return 0;
  else if ( k == 1 )
    return 1;

  nums[0] = 0;
  nums[1] = 1;

  for ( i = 2; i <= k; i++ )
    nums[i] = nums[i-1] + nums[i-2];
  
  return nums[k];
}

module_init(fibonacci_init);
module_exit(fibonacci_exit);
