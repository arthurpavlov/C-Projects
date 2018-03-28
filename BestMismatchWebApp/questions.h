#ifndef QUESTIONS_H
#define QUESTIONS_H

#define MAX_LINE 256

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct str_node {
	char *str;
	int fd;
	struct str_node *next;
	int *useranswers;
} Node;

Node * get_list_from_file (char *input_file_name);
Node* get_user(Node *head, char *username);
int check_list_for_user(Node *head, char *username);
int check_list_for_fd(Node *head, int user_fd);
int get_question_count(Node *head);
void add_user_name_to_list(Node *head, char *username);
void print_user_answers(Node *user, int numanswers);
void print_list (Node *current);
void free_list (Node *head);
void free_user_list (Node *head);

#endif
