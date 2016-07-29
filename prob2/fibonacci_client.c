#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  char buf[BUFSIZ] = {'\0'};
  int i;
  int k;
  int fd;

  if ( argc != 2 || (k = atoi(argv[1])) < 0 ) 
  {
    fprintf(stderr, "Invalid arguments\n");
    return -1;
  }

  if ( (fd = open("/dev/fibonacci", O_RDONLY)) == -1 )
  {
    fprintf(stderr, "Unable to open /dev/fibonacci\n");
    return -1;
  }

  printf("The 0 to %dth fibonacci nums as returned /dev/fibonacci are:\n", k);
  for ( i = 0; i < k; i++ )
  {
    if ( pread(fd, buf, BUFSIZ, i) == -1 )
    {
      fprintf(stderr, "Error reading /dev/fibonacci: %s\n", strerror(errno));
      return -1;
    } 

    printf("%s ", buf);
  }
  printf("\n");

  close(fd);

  return 0;
}
