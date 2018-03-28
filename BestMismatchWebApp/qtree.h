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
Node *get_mismatch(QNode *current, int usersanswers[], int depth);
void add_user(QNode *current, int list[], char *username, int usersanswers[]);
void add_to_leaf(QNode *current, int answer, char *username, int usersanswers[]);
void print_qtree (QNode *parent, int level);
void print_users (Node *parent);
void free_tree(QNode *current);


#endif
