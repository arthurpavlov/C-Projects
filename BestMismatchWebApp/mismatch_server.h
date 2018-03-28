#ifndef MISMATCH_SERVER_H
#define MISMATCH_SERVER_H

#include "mismatch_server_utils.h"

#ifndef PORT
	#define PORT 52737
#endif

#define MAX_NAME 128
#define BUFFER_SIZE 1024

typedef struct client {
	int fd; //Client's dedicated file descriptor
	int taken_test; // 1/0 representing client's completion of test
	int *answers; //List of client's anwers
	Node *my_questions; //List of questions client still needs to answer
	int questions_answered; //The number of questions client has answered
	char name [MAX_NAME]; //The name of the client
	char buf [BUFFER_SIZE]; //Client's designated buffer for server interaction
	int status; //Status represents client's position in program (1 = Command Select, 2 = Taking Test)
	int inbuf; //Index to position in buffer
	struct client *next; //Link to next client in list
} Client;

#endif
