#include <linux/netlink.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "simple_db.h"

void gen_message(char *key, char *val, char *buf);

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
  key = calloc(SIMPLE_DB_KEY_LEN, sizeof(char));
  value = calloc(SIMPLE_DB_VAL_LEN, sizeof(char));
  snd_str = NLMSG_DATA(snd_nlh);
  rcv_str = NLMSG_DATA(rcv_nlh);

  // read user commands
  while ( printf("Do you want to SET or GET a variable?\n")\
          && fgets(line, BUFSIZ, stdin) )
  {
    int i;

    // setup to send a SET message
    if ( strcasecmp(line, "set\n") == 0 )
    {
      printf("Enter key:\n");
      fgets(key, SIMPLE_DB_KEY_LEN, stdin);
      key[strlen(key)-1] = '\0';
      printf("Enter value:\n");
      fgets(value, SIMPLE_DB_VAL_LEN, stdin);
      value[strlen(value)-1] = '\0';
      gen_message(key, value, snd_str);
    }
    // setup to send a GET message
    else if ( strcasecmp(line, "get\n") == 0 )
    {      
      printf("Enter key:\n");
      fgets(key, SIMPLE_DB_KEY_LEN, stdin);
      key[strlen(key)-1] = '\0';
      gen_message(key, NULL, snd_str);
    }
    else
    {
      printf("Not a valid command.\n");
      continue;
    }
 
    // send message
    if ( sendmsg(fd, &snd_msg, 0) == -1 )
      fprintf(stderr, "Error: %s\n", strerror(errno));
    else
    {
      printf("Sent netlink message:\n");
      for ( i = 0; i < SIMPLE_DB_MSG_LEN; i++ )
        printf("%c", snd_str[i]);
      printf("\n\n");
    }

    // recieve message back
    if ( strcasecmp(line, "get\n") == 0 )
    {
      printf("Waiting for value...\n");
      if ( recvmsg(fd, &rcv_msg, 0) == -1 )
        fprintf(stderr, "Error: %s\n", strerror(errno));
      else
      {

        printf("Received netlink message:\n");
        for ( i = 0; i < SIMPLE_DB_MSG_LEN; i++ )
          printf("%c", rcv_str[i]);
        printf("\n\n");

        if ( rcv_str[0] == '\0' )
          printf("No value for key.\n");
        else
          printf("Value for key %s == %s\n", key, rcv_str);
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

void gen_message(char *key, char *val, char *buf)
{
  int i;

  memset(buf, 0, SIMPLE_DB_MSG_LEN);

  if ( val )
    buf[0] = 'S';
  else
    buf[0] = 'R';
  
  for ( i = 0; i < SIMPLE_DB_KEY_LEN; i++ )
    buf[i+SIMPLE_DB_TYP_LEN] = key[i];

  if ( val )
    for ( i = 0; i < SIMPLE_DB_VAL_LEN; i++ )
      buf[i+SIMPLE_DB_TYP_LEN+SIMPLE_DB_KEY_LEN] = val[i];
}
