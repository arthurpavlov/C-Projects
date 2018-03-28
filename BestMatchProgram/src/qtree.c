#include "qtree.h"

QNode *add_next_level (QNode *current, Node *list_node) {
	int str_len;
	
	str_len = strlen (list_node->str);
	current = (QNode *) calloc (1, sizeof(QNode));

	current->question =  (char *) calloc (str_len +1, sizeof(char ));
	strncpy ( current->question, list_node->str, str_len );
	current->question [str_len] = '\0';  
	current->node_type = REGULAR;
	
	if (list_node->next == NULL) {
		current->node_type = LEAF;
		return current;
	}
	
	current->children[0].qchild = add_next_level ( current->children[0].qchild, list_node->next);
	current->children[1].qchild = add_next_level ( current->children[1].qchild, list_node->next);

	return current;
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


Node *search_user(QNode *current, char *username){
	if((current->node_type == LEAF)){
		Node *lhead = malloc(sizeof(Node)*1);
		lhead = current->children[0].fchild;
		Node *rhead = malloc(sizeof(Node)*1);
		rhead = current->children[1].fchild;

		if((check_list_for_user(lhead, username) == 1)){
			return lhead;
		}
		else if((check_list_for_user(rhead, username) == 1)){
			return rhead;
		}
		else{
			return NULL;
		}
	}
	else{
		Node *left = search_user(current->children[0].qchild, username);
		Node *right = search_user(current->children[1].qchild, username);
		if((left != NULL)){
			return left;
		}
		else if((right != NULL)){
			return right;
		}
		else{
			return NULL;
		}
	}
}


void add_to_leaf(QNode *current, int answer, char *username){
	char *name = malloc(MAX_LINE);
	strcpy(name, username);
	Node *new = malloc(sizeof(Node)*1);
	new->str = name;
	new->next = NULL;
	if((answer == 1)){
		if((current->children[1].fchild == NULL)){
			current->children[1].fchild = new;
		}
		else{
			Node *temp = current->children[1].fchild;
			while(temp->next != NULL){
				temp = temp->next;
			}
			temp->next = new;
		}
	}
	else{
		if((current->children[0].fchild == NULL)){
			current->children[0].fchild = new;
		}
		else{
			Node *temp = current->children[0].fchild;
			while(temp->next != NULL){
				temp = temp->next;
			}
			temp->next = new;
		}
	}
}


void add_user(QNode *current, int list[], char *username){
	if((current->node_type == LEAF)){
		int i = list[0];
		add_to_leaf(current, i, username);
	}
	else{
		int *answer = (int *)malloc(MAX_LINE);
		answer = &(list[0]);
		int* new_list = list + 1;
		if((*answer == 0)){
			QNode * next_node = current->children[0].qchild;
			add_user(next_node, new_list, username);
		}
		else{
			QNode * next_node = current->children[1].qchild;
			add_user(next_node, new_list, username);
		}

	}
}


void print_qtree (QNode *parent, int level) {
	int i;
	for (i=0; i<level; i++)
		printf("\t");
	
	printf ("%s type:%d\n", parent->question, parent->node_type);
	if(parent->node_type == REGULAR) {
		print_qtree (parent->children[0].qchild, level+1);
		print_qtree (parent->children[1].qchild, level+1);
	}
	else { //leaf node
		for (i=0; i<(level+1); i++)
			printf("\t");
		print_users (parent->children[0].fchild);
		for (i=0; i<(level+1); i++)
			printf("\t");
		print_users (parent->children[1].fchild);
	}
}

void print_users (Node *parent) {
	if (parent == NULL)
		printf("NULL\n");
	else {
		printf("%s, ", parent->str);
		while (parent->next != NULL) {
			parent = parent->next;
			printf("%s, ", parent->str);
		}
		printf ("\n");
	}
}
