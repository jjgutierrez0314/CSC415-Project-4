#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAXBUFLEN 5000

int main(int argc, char** argv){

  char source[MAXBUFLEN + 1];
  FILE *fp = fopen(argv[1], "r");
  if (fp != NULL)
  {
    size_t newLen = fread(source, sizeof(char), MAXBUFLEN, fp);
    if (ferror(fp) != 0)
    {
      fputs("Error reading file", stderr);
    }
    else
    {
      source[newLen++] = '\0'; /* Just to be safe. */
    }
    fclose(fp);
  }
  printf("%s", source);
  
}