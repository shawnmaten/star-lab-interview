#include <linux/module.h>
#include <net/sock.h> 
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>

#include "simple_db.h"

typedef struct blob
{
  char key[SIMPLE_DB_KEY_LEN];
  char value[SIMPLE_DB_VAL_LEN];
} 
blob_t;

static void recv_func(struct sk_buff *buf);
blob_t *find_blob(char *key);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shawn Aten");
MODULE_DESCRIPTION("A simple linux module for a db accessed via netlink.");
  
static struct sock *nlsock;

static blob_t *blobs = NULL;

static int num_blobs = 0;

static int __init simple_db_init(void) 
{
  struct netlink_kernel_cfg cfg = 
  {
    .input = recv_func
  };

  nlsock = netlink_kernel_create(&init_net, NETLINK_UNUSED, &cfg);
  if ( nlsock == NULL )
  {
    printk(KERN_INFO "Simple DB: Failed to create sock\n");
    return -EIO;
  }
    
  printk(KERN_INFO "Simple DB: Initialized\n");

  return 0;
}

static void __exit simple_db_exit(void)
{
  netlink_kernel_release(nlsock);
  kfree(blobs);
  printk(KERN_INFO "Simple DB: Exited\n");
}

static void recv_func(struct sk_buff *skb_in)
{
  int pid;
  int len;

  char type;
  char *message;
  char *key;
  char *value;

  struct nlmsghdr *rcv_nlh;

  blob_t *blob;
  
  rcv_nlh = (struct nlmsghdr *)skb_in->data;
  pid = rcv_nlh->nlmsg_pid;
  len = rcv_nlh->nlmsg_len;

  message = (char *)NLMSG_DATA(rcv_nlh);
  type = message[0];
  key = message + SIMPLE_DB_TYP_LEN;
  value = key + SIMPLE_DB_KEY_LEN;

  printk(KERN_INFO "Simple DB: Recv %d bytes from %d\n", len, pid);
  
  if ( type == 'S' )
  {
    printk(KERN_INFO "Simple DB: Type: S %s:%s\n", key, value);

    blob = find_blob(key);
    if ( !blob )
    {
      num_blobs++;
      blobs = krealloc(blobs, num_blobs * sizeof(blob_t), GFP_KERNEL);
      blob = blobs+(num_blobs-1);
      memcpy(blob->key, key, SIMPLE_DB_KEY_LEN);
    }
    memcpy(blob->value, value, SIMPLE_DB_VAL_LEN);
  }
  else if ( type == 'R' )
  {
    struct sk_buff *skb_out;

    struct nlmsghdr *snd_nlh;
    
    printk(KERN_INFO "Simple DB: Type: R %s\n", key);

    skb_out = nlmsg_new(SIMPLE_DB_MSG_LEN, 0);
    if (!skb_out) {
        printk(KERN_ERR "Simple DB: Failed to allocate new skb\n");
        return;
    }
    snd_nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, SIMPLE_DB_MSG_LEN, 0);
    NETLINK_CB(skb_out).dst_group = 0;
    memset(NLMSG_DATA(snd_nlh), 0, SIMPLE_DB_MSG_LEN);

    blob = find_blob(key);
    if ( blob )
      strcpy(NLMSG_DATA(snd_nlh), blob->value);
    
    /*
    struct sk_buff *snd_buf;
    struct nlmsghdr *snd_hdr;
    
    snd_buf = alloc_skb(NLMSG_SPACE(SIMPLE_DB_VAL_LEN), GFP_KERNEL);
    snd_hdr = (struct nlmsghdr *)snd_buf->data;
    snd_hdr->nlmsg_len = NLMSG_SPACE(SIMPLE_DB_VAL_LEN);
    snd_hdr->nlmsg_pid = 0;
    snd_hdr->nlmsg_flags = 0;
    // NETLINK_CB(snd_buf).dst_pid = pid;
    
    message = NLMSG_DATA(snd_hdr);
    strcpy(message, "NOT IMPLEMENTED");
    */

    if ( netlink_unicast(nlsock, skb_out, pid, MSG_DONTWAIT) < 0 )
      printk(KERN_INFO "Simple DB: Unable to send response\n");
    else
      printk(KERN_INFO "Simple DB: Sent: %s\n", (char *)NLMSG_DATA(snd_nlh));
  }
}

blob_t *find_blob(char *key)
{
  int i;

  for ( i = 0; i < num_blobs; i++ )
    if ( strncmp(blobs[i].key, key, SIMPLE_DB_KEY_LEN) == 0 )
      return blobs+i;

  return NULL;
}

module_init(simple_db_init);
module_exit(simple_db_exit);
