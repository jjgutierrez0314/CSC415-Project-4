/*************                                                                 

  File: client.c -- a stream socket client demo   
  Compile: gcc client.c -o client
  Run: ./client [server] [message]

  Description: adapted from Brian "Beej Jorgensen" Hall
  https://beej.us/guide/bgnet/html/#a-simple-stream-client
                      
  [server] is domain name of server
  [message] is a string
  IPv4 only!
  Client sends message to server
  Receives reversed message from server

*************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>

#include <arpa/inet.h>

#define PORT "5001" // the port client will be connecting to

#define MAXDATASIZE 5000 // max number of bytes we can get at once

int main(int argc, char *argv[])
{
  int sockfd, numbytes;
  char buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p; //hints narrows down the options we have
  int rv;
  char s[INET_ADDRSTRLEN];
  FILE *fp;

  if (argc != 3)
  {
    fprintf(stderr, "usage: client hostname message\n");
    exit(1);
  }

  /* get info for server */
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and connect to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    /* try to create socket */
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1)
    {
      perror("client: socket");
      continue;
    }

    /* connect socket to server */
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      perror("client: connect");
      close(sockfd);
      continue;
    }
    break;
  }

  if (p == NULL)
  {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  struct sockaddr_in *sa = (struct sockaddr_in *)p->ai_addr;
  inet_ntop(AF_INET, &(sa->sin_addr), s, sizeof(s));
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure

  /* send message to server */
  char *mess = argv[2];
  fp = fopen(mess, "r");
  numbytes = write(sockfd, fp, strlen(fp) + 1);
  if (numbytes != strlen(mess) + 1)
  {
    perror("write");
    close(sockfd);
    exit(0);
  }

  /* receive message from server */
  numbytes = read(sockfd, buf, MAXDATASIZE - 1);

  buf[numbytes] = '\0';
  printf("client: received '%s'\n", buf);

  close(sockfd);
  return 0;
}
