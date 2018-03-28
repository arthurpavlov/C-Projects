#ifndef MISMATCH_SERVER_UTILS_H
#define MISMATCH_SERVER_UTILS_H

#define _XOPEN_SOURCE

#include "qtree.h"
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Bellow structure is imported from https://www.cs.cmu.edu/afs/cs/academic/class/115213-f99/www/class26/tcpserver.c
 * */
#if 0
/*
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr;
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

int check_answer_format(char *answer);
int find_network_newline(char *buffer, int in_buf);
int check_command_format(char *user_cmd);
int is_quit(char * buffer);
int check_alphanum(char *username);
int check_username(char * username);
char *get_ip_from_fd(int input_fd);

#endif
