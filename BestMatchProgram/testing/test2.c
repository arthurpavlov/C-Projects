#include "qtree.h"

void show_list(int items[]){
	int i;
	int n = sizeof(items);
	for(i = 0; i< n; i++){
		printf("%d\n", items[i]);
	}
}

int get_number_of_questions(Node *head){
	int count = 0;
	while(head != NULL){
		count++;
		head = head->next;
	}
	return count;
}



int main (int argc, char ** argv) {
    QNode *root = NULL;
	Node *q_list;
    
    if (argc < 2) {
        printf ("To run the program ./test2 <name of input file>\n");
        return 1;
    }
    
    q_list = get_list_from_file (argv[1]);
    
    if (q_list == NULL) {
        printf ("The list is empty\n");
        return 1;
    }
    
    root = add_next_level (root, q_list);


    int *list_of_answers;
    list_of_answers = (int *)malloc(10000*sizeof(argv));

    int i;
	for(i = 3; i < argc; i++){
		int *answer = (int *)malloc(MAX_LINE);
		int item = atoi(argv[i]);
		answer = &(item);
		list_of_answers[i - 3] = *answer;
	}

	int number_of_answers = argc - 3;
	int number_of_questions = get_number_of_questions(q_list);

	if((number_of_questions != number_of_answers)){
		printf("%s\n", "Incorrect number of answers");
		exit(0);
	}

	char *user_name;
	user_name = malloc(MAX_LINE);
	user_name = argv[2];

	add_user(root, list_of_answers, user_name);

	print_qtree (root, 0);
    
    return 0;
}


