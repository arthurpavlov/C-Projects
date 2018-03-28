#include "qtree.h"
#include <ctype.h>

void print_greetings () {
	printf ("----------------------------------------------\n");
	printf ("Friend recommender system. Find people who are just like you!\n");
	printf ("CSC209 fall 2016 team. All rights reserved\n");
	printf ("----------------------------------------------\n");
}

void print_matches (Node *head, char *username) {
	while (head != NULL){
		if((strcmp(head->str, username) != 0)){
			printf("%s\n", head->str);
		}
		head = head->next;
	}
}

int check_alphanum(char *username){
	int answer = 1;
	int i;
	for(i=0; i< strlen(username); i++){
		if((isalnum(username[i])== 0)){
			answer = 0;
		}
	}
	return answer;

}

char *check_username(){
	char *username = malloc(sizeof(char)*200);
	printf("%s", "What is your username?\n");
	scanf("%s", username);
	int usernamelen = strlen(username);
	if((username[0] == 'q')){
		return username;
	}
	else if((check_alphanum(username) == 0 || usernamelen < 8 || usernamelen > 128)){
		return NULL;
	}
	return username;
}

int check_answer(char *answer){
	if((strlen(answer)>3)){
		return 0;
	}
	else if((answer[0]!= 'y' && answer[0]!= 'n' && answer[0] != 'q')){
		return 0;
	}
	else{
		return 1;
	}
}


int main (int argc, char **argv) {
	
	QNode *root = NULL;

	Node * interests = NULL;

	if (argc < 2) {
        printf ("To run the program ./categorizer <name of input file>\n");
        return 1;
    }
    interests = get_list_from_file ( argv[1]);
    if (interests == NULL)
    	return 1;

    print_greetings();
	root = add_next_level (root,  interests);
	int terminate = 0;
	while(terminate == 0){
		interests = get_list_from_file ( argv[1]);
		//check username
		char *username = malloc(sizeof(char)*200);
		int userfound = 0;
		while(userfound == 0){
			username = check_username();
			if((username == NULL )){
				printf("%s", "Invalid Username!\n");
			}
			else if((username[0] == 'q')){
				printf("%s", "Goodbye!\n");
				exit(0);
			}
			else if((search_user(root, username) != NULL)){
				//user already exists, return matches
				Node * matches = search_user(root, username);
				if((matches->next == NULL && strcmp(matches->str, username)==0)){
					printf("%s", "\n");
					printf("%s", "You have no matches! Try Again Later!\n");
					printf("%s", "\n");
				}
				else{
					printf("%s", "\n");
					printf("%s", "You have been matched with...\n");
					print_matches(matches, username);
					printf("%s", "\n");
				}
			}
			else{
				userfound = 1;
			}
		}
		//check answers
		int *answers = malloc(30 * sizeof(int));
		int i = 0;
		while(interests!= NULL){
			int *finalanswer = malloc(sizeof(char)*200);
			int found = 0;
			while(found == 0){
				char *answer = malloc(sizeof(char)*200);
				printf("%s%s%s", "Do you like ", interests->str, "? (y/n)\n");
				scanf("%s", answer);
				if((check_answer(answer) == 1)){
					if((answer[0] == 'y')){
						*finalanswer = 1;
					}
					else if((answer[0] == 'n')){
						*finalanswer = 0;
					}
					else if((answer[0] == 'q')){
						printf("%s", "Goodbye!\n");
						exit(0);
					}
					found = 1;
				}
				else{
					printf("%s", "Invalid Answer!\n");
				}
			}
			answers[i] = *finalanswer;
			i++;
			interests = interests->next;
		}
		add_user(root, answers, username);
		//print_qtree (root, 0);
		Node * matches = search_user(root, username);
		if((matches->next == NULL && strcmp(matches->str, username)==0)){
			printf("%s", "\n");
			printf("%s", "You have no matches! Try Again Later!\n");
			printf("%s", "\n");
		}
		else{
			printf("%s", "\n");
			printf("%s", "You have been matched with...\n");
			print_matches(matches, username);
			printf("%s", "\n");
		}
	}
	return 0;
}

