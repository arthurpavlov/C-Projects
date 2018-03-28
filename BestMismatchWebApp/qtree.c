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

Node *search_user(QNode *current, char *username){
	if((current->node_type == LEAF)){
		Node *lhead;
		lhead = current->children[0].fchild;
		Node *rhead;
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

Node *get_mismatch(QNode *current, int usersanswers[], int depth){
	if(depth == 1){
		if(usersanswers[0] == 0){
			return current->children[1].fchild;
		}
		else{
			return current->children[0].fchild;
		}
	}
	else{
		if(usersanswers[0] == 0){
			return get_mismatch(current->children[1].qchild, usersanswers + 1, depth - 1);
		}
		else{
			return get_mismatch(current->children[0].qchild, usersanswers + 1, depth - 1);
		}
	}
}


void add_to_leaf(QNode *current, int answer, char *username, int usersanswers[]){
	char *name = malloc(MAX_LINE);
	strcpy(name, username);
	Node *new = malloc(sizeof(Node)*1);
	new->str = name;
	new->next = NULL;
	new->useranswers = usersanswers;

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


void add_user(QNode *current, int list[], char *username, int usersanswers[]){
	if((current->node_type == LEAF)){
		int i = list[0];
		add_to_leaf(current, i, username, usersanswers);
	}
	else{
		int* new_list = list + 1;
		if((list[0] == 0)){
			QNode * next_node = current->children[0].qchild;
			add_user(next_node, new_list, username, usersanswers);
		}
		else{
			QNode * next_node = current->children[1].qchild;
			add_user(next_node, new_list, username, usersanswers);
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

void free_tree(QNode *parent){
	if(parent->node_type == LEAF){
		free_user_list(parent->children[0].fchild);
		free_user_list(parent->children[1].fchild);
		free(parent->question);
		free(parent);
	}
	else{
		free_tree(parent->children[0].qchild);
		free_tree(parent->children[1].qchild);
		free(parent->question);
		free(parent);
	}
}
