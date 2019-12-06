/*************

  File: server.c -- a stream socket server demo
  Compile: gcc server.c -o server
  Run: ./server

  Description: adapted from Brian "Beej Jorgensen" Hall
  https://beej.us/guide/bgnet/html/#a-simple-stream-server

  IPv4 only!
  Server receives string from client
  Server reverses string, sends back to client

*************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

#define PORT "5001" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

#define MAXCHARS 5000

struct FileInfo
{
  char *name;   /* name of file */
  int numLines; /* number of lines in file */
  int numWords; /* number of words in file */
  int numChars; /* number of characters in file */
} fileInfo;

int main(void)
{
  int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  int yes = 1;
  char s[INET_ADDRSTRLEN];
  int rv;

  //added stuff from fileStats.c
  FILE *fp;
  struct FileInfo *info;                        /* array of counts for each file */
  int numLines = 0, numWords = 0, numChars = 0; /* total counts */
  char inString[MAXCHARS];
  char *rs;
  info = (struct FileInfo *)malloc(sizeof(struct FileInfo));

  /* get info for self, to set up socket */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    /* try to create socket */
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1)
    {
      perror("server: socket");
      continue;
    }

    /* set socket options so socket can be reused */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1)
    {
      perror("setsockopt");
      exit(1);
    }

    /* bind socket to port number */
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL)
  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1)
  {
    perror("listen");
    exit(1);
  }

  printf("server: waiting for connections...\n");

  while (1)
  { // main accept() loop
    sin_size = sizeof their_addr;
    /* block until request arrives */
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1)
    {
      perror("accept");
      continue;
    }

    /* get information about connecting client */
    struct sockaddr_in *sa = (struct sockaddr_in *)&their_addr;
    inet_ntop(AF_INET, &(sa->sin_addr), s, sizeof(s));
    printf("server: got connection from %s\n", s);

    if (!fork())
    {                /* create child to handle new client */
      close(sockfd); /* child doesn't need the listener */
      char s[80];
      /* read string from client */
      int numBytes = read(new_fd, fp, 80);
      if (numBytes > 80)
      {
        perror("read");
        close(new_fd);
        exit(0);
      }
      int i = 0;
      /* Compute counts */
      info[i].name = (char *)malloc(MAXCHARS * sizeof(char));
      strncpy(info[i].name, argv[i + 1], MAXCHARS);
      info[i].numLines = 0;
      info[i].numWords = 0;
      info[i].numChars = 0;

      numBytes = write(new_fd, s, strlen(s) + 1);
      if (numBytes != strlen(s) + 1)
      {
        perror("write");
        close(new_fd);
        exit(0);
      }
      close(new_fd);
      exit(0);
    }
    close(new_fd); /* parent doesn't need this */
  }

  return 0;
}

int countWords(char *inS)
{
  char *token;
  int numTokens = 0;
  int i = 0;

  for (i = 1; i < strlen(inS); i++)
  {
    if ((isalnum(inS[i - 1]) || ispunct(inS[i - 1])) && (inS[i] == ' '))
    {
      numTokens++;
    }
  }

  if (isalnum(inS[strlen(inS) - 2]) || ispunct(inS[strlen(inS) - 2]))
  {
    numTokens++;
  }
  return numTokens;
}
