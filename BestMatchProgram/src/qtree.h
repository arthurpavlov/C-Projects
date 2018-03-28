#ifndef QTREE_H
#define QTREE_H
#include "questions.h"

typedef enum {
    REGULAR, LEAF
} NodeType;

union Child {
	struct str_node *fchild;
	struct QNode *qchild;
} Child;

typedef struct QNode {
	char *question;
	NodeType node_type;
	union Child children[2];
} QNode;

QNode *add_next_level (QNode *current, Node * list_node);
Node *search_user(QNode *current, char *username);
int check_list_for_user(Node *head, char *username);
void print_qtree (QNode *parent, int level);
void print_users (Node *parent);
void add_user(QNode *current, int list[], char *username);
void add_to_leaf(QNode *current, int answer, char *username);

#endif
