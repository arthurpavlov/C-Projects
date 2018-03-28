#include "questions.h"


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

void add_user_name_to_list(Node *head, char *username){
	Node *new = malloc(sizeof(Node)*1);
	new->str = username;
	new->next = NULL;
	if((head == NULL)){
		head = new;
	}
	else{
		Node *head_pntr = head;
		while(head_pntr->next != NULL){
			head_pntr = head_pntr->next;
		}
		head_pntr->next = new;
	}
//	printf("head %s\n", head->str);
}




int get_question_count(Node *head){
	int count = 0;
	while(head != NULL){
		count++;
		head = head->next;
	}
	return count;
}


Node* get_user(Node *head, char *username){
	while(head != NULL){
		if((strcmp(head->str, username) == 0)){
			return head;
		}
		head = head->next;
	}
	return NULL;
}

int check_list_for_user(Node *head, char *username){
	while(head!=NULL){
		if((strcmp(head->str, username) == 0)){
			return 1;
		}
		head = head->next;
	}
	return 0;
}

int check_list_for_fd(Node *head, int user_fd){
	while(head!=NULL){
		if((head->fd == user_fd)){
			return 1;
		}
		head = head->next;
	}
	return 0;
}




void print_user_answers(Node *user, int numanswers){
	int i = 0;
	while(i < numanswers)	{
		printf("%d\n", user->useranswers[i]);
		i++;
	}

}


void print_list (Node *head) {
	while (head != NULL){
		printf("%s\n", head->str);
		printf("%d\n", head->fd);
		head = head->next;
	}
}

void free_list(Node *head){
	Node *curr;
	while ((curr = head) != NULL){
		free(curr->str);
		free(curr);
		head = head->next;
	}
}

void free_user_list(Node *head){
	Node *curr;
	while ((curr = head) != NULL){
		free(curr->str);
		free(curr->useranswers);
		free(curr);
		head = head->next;
	}
}

