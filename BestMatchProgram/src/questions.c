#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "questions.h"
#include <ctype.h>


Node * get_list_from_file (char *input_file_name) {
	Node * head = NULL;
	Node * tail = NULL;
	char line[MAX_LINE];
	FILE *fp;
	fp =fopen(input_file_name, "r");
	while(fgets(line, MAX_LINE, fp)!=NULL){
		line [strcspn (line, "\r\n")] = '\0';
		if((head == NULL)){
			char *firstinterest = malloc(MAX_LINE);
			strcpy(firstinterest, line);
			Node *new = malloc(sizeof(Node)*1);
			new->str = firstinterest;
			new->next = NULL;
			head = new;
			tail = new;
		}
		else{
			char *restinterests = malloc(MAX_LINE);
			Node *new = malloc(sizeof(Node)*1);
			strcpy(restinterests, line);
			new->str = restinterests;
			new->next = NULL;
			tail->next = new;
			tail = new;
		}
	}
	fclose(fp);
	return head;
}

void print_list (Node *head) {
	while (head != NULL){
		printf("%s\n", head->str);
		head = head->next;
	}
}
