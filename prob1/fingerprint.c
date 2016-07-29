#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <openssl/sha.h>
#include <stdio.h>

#define ATTR_COUNT  4

int get_attr(char *file_str, char *regex_str, char **attr);

// cat /proc/cpuinfo
// cat /sys/class/dmi/id/product_uuid
// cat /sys/class/net/eth0/address
// cat /proc/self/cgroup
// 109 lines

int main(int argc, char **argv) 
{
  char *file_strs[ATTR_COUNT] = 
  {
    "/proc/cpuinfo",
    "/sys/class/net/eth0/address",
    "/sys/class/dmi/id/product_uuid",
    "/proc/self/cgroup"
  };

  char *regex_strs[ATTR_COUNT] = 
  {
    "^model name	: (.**)$",
    NULL,
    NULL,
    "^.**docker/(.**)$"
  };

  char *attr_labels[ATTR_COUNT] = 
  {
    "CPU Model",
    "MAC",
    "Board UUID",
    "Container ID"
  };

  char *attrs[ATTR_COUNT];

  int i;

  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;

  for ( i = 0; i < ATTR_COUNT; i++)
    get_attr(file_strs[i], regex_strs[i], &attrs[i]);

  SHA256_Init(&sha256);

  for ( i = 0; i < ATTR_COUNT; i++)
    if ( attrs[i] )
      SHA256_Update(&sha256, attrs[i], strlen(attrs[i]));

  SHA256_Final(hash, &sha256);

  for ( i = 0; i < ATTR_COUNT; i++)
    if ( attrs[i] )
      printf("Attribute #%d %s:\n  %s\n", i, attr_labels[i], attrs[i]);

  printf("\nSystem Fingerprint:\n  ");
  for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
    printf("%02x", hash[i]);
  printf("\n");

  for ( i = 0; i < ATTR_COUNT; i++)
    if ( attrs[i] )
      free(attrs[i]);

  return 0;
}

int get_attr(char *file_str, char *regex_str, char **attr)
{
  FILE *file;

  size_t n;

  char *line;

  regex_t regex;
  regmatch_t matches[2];

  int result;
  int match_len;

  *attr = NULL;
  file = fopen(file_str, "r");

  if ( !file )
    return 0;

  n = 0;
  line = NULL;
  result = -1;

  if ( regex_str && regcomp(&regex, regex_str, REG_EXTENDED) == 0 ) 
  {
    while ( getline(&line, &n, file) != -1 ) 
    {
      result = regexec(&regex, line, 2, matches, 0);
      if ( result == 0) 
      {
        match_len = matches[1].rm_eo - matches[1].rm_so - 1;
        *attr = malloc(sizeof(char) * (match_len + 1));
        strncpy(*attr, line+matches[1].rm_so, match_len);
        (*attr)[match_len] = '\0';
        free(line);
        break;
      }
    }

    regfree(&regex);

  } 
  else if ( getline(&line, &n, file) != -1 ) 
  {
    line[strlen(line) - 1] = '\0';
    *attr = line;
  }

  fclose(file);

  if ( !*attr ) 
  {
    free(line);
    return 0;
  } 
  else {
    return 1;
  }

}
