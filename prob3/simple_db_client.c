#include <linux/netlink.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "simple_db.h"

void gen_message(char type, char *key, char *val, char *buf);

int main(void)
{
  int fd;
  
  struct sockaddr_nl src_addr;
  struct sockaddr_nl dst_addr;
  
  struct nlmsghdr *snd_nlh; 
  struct nlmsghdr *rcv_nlh; 
  
  struct iovec snd_iov;
  struct iovec rcv_iov;
  
  struct msghdr snd_msg;
  struct msghdr rcv_msg;

  char line[BUFSIZ];
  char *key;
  char *value;
  char *snd_str;
  char *rcv_str;

  // setup headers
  memset(&src_addr, 0, sizeof(src_addr));
  src_addr.nl_family = AF_NETLINK;
  src_addr.nl_pid = getpid();

  memset(&dst_addr, 0, sizeof(dst_addr));
  dst_addr.nl_family = AF_NETLINK;

  // for sending
  snd_nlh = (struct nlmsghdr *) calloc(NLMSG_SPACE(SIMPLE_DB_MSG_LEN), 1);
  snd_nlh->nlmsg_len = NLMSG_SPACE(SIMPLE_DB_MSG_LEN);
  snd_nlh->nlmsg_pid = getpid();
  
  snd_iov.iov_base = (void *)snd_nlh;
  snd_iov.iov_len = snd_nlh->nlmsg_len;

  snd_msg.msg_name = (void *)&dst_addr;
  snd_msg.msg_namelen = sizeof(dst_addr);
  snd_msg.msg_iov = &snd_iov;
  snd_msg.msg_iovlen = 1;

  // for recieving
  rcv_nlh = (struct nlmsghdr *) calloc(NLMSG_SPACE(SIMPLE_DB_MSG_LEN), 1);
  rcv_nlh->nlmsg_len = NLMSG_SPACE(SIMPLE_DB_MSG_LEN);
  rcv_nlh->nlmsg_pid = 0;
  
  rcv_iov.iov_base = (void *)rcv_nlh;
  rcv_iov.iov_len = rcv_nlh->nlmsg_len;

  rcv_msg.msg_name = (void *)&src_addr;
  rcv_msg.msg_namelen = sizeof(src_addr);
  rcv_msg.msg_iov = &rcv_iov;
  rcv_msg.msg_iovlen = 1;

  // setup socket
  if ( (fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_UNUSED)) == -1)
    fprintf(stderr, "Error: %s\n", strerror(errno));
  bind(fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

  // setup char buffers
  key = malloc(SIMPLE_DB_KEY_LEN);
  value = malloc(SIMPLE_DB_VAL_LEN);
  snd_str = NLMSG_DATA(snd_nlh);
  rcv_str = NLMSG_DATA(rcv_nlh);

  // read user commands
  while ( printf("Do you want to SET or GET a variable?\n")\
          && fgets(line, BUFSIZ, stdin) )
  {
    int i;

    char type;

    memset(key, 0, SIMPLE_DB_KEY_LEN);
    memset(value, 0, SIMPLE_DB_VAL_LEN);

    // setup to send a SET message
    if ( strcasecmp(line, "set\n") == 0 )
    {
      printf("Enter key:\n");
      fgets(key, SIMPLE_DB_KEY_LEN, stdin);
      key[strlen(key)-1] = '\0';
      printf("Enter value:\n");
      fgets(value, SIMPLE_DB_VAL_LEN, stdin);
      value[strlen(value)-1] = '\0';
      type = 'S';
    }
    // setup to send a GET message
    else if ( strcasecmp(line, "get\n") == 0 )
    {      
      printf("Enter key:\n");
      fgets(key, SIMPLE_DB_KEY_LEN, stdin);
      key[strlen(key)-1] = '\0';
      type = 'R';
    }
    else
    {
      printf("Not a valid command.\n");
      continue;
    }
    
    gen_message(type, key, value, snd_str);
 
    // send message
    if ( sendmsg(fd, &snd_msg, 0) == -1 )
      fprintf(stderr, "Error: %s\n", strerror(errno));

    // recieve message back
    if ( strcasecmp(line, "get\n") == 0 )
    {
      printf("Waiting for value...\n");
      if ( recvmsg(fd, &rcv_msg, 0) == -1 )
        fprintf(stderr, "Error: %s\n", strerror(errno));
      else
      {
        int non_null_bytes = 0;
        
        for ( i = 0; i < SIMPLE_DB_MSG_LEN; i++ )
          if ( rcv_str[i] != '\0' )
            non_null_bytes++;

        if ( non_null_bytes )
        {
          printf("Value for key %s:\n", key);
          for ( i = 0; i < SIMPLE_DB_MSG_LEN; i++ )
            printf("%c", rcv_str[i]);
          printf("\n");
        }
        else
          printf("Key %s not found.\n", key);

      }
    }
  }

  close(fd);
  free(snd_nlh);
  free(rcv_nlh);
  free(key);
  free(value);

  return 0;
}

void gen_message(char type, char *key, char *val, char *buf)
{
  int i;

  buf[0] = type;
  
  for ( i = 0; i < SIMPLE_DB_KEY_LEN; i++ )
    buf[i+SIMPLE_DB_TYP_LEN] = key[i];

  for ( i = 0; i < SIMPLE_DB_VAL_LEN; i++ )
    buf[i+SIMPLE_DB_TYP_LEN+SIMPLE_DB_KEY_LEN] = val[i];
}
