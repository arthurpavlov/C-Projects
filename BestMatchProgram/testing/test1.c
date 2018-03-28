#include <stdio.h>
#include "questions.h"

int main (int argc, char ** argv) {
    Node * interests;

    if (argc < 2) {
        printf ("To run the program ./test1 <name of input file>\n");
        return 1;
    }
    
    interests = get_list_from_file(argv[1]);
    if (interests == NULL) {
        printf ("The list is empty\n");
        return 1;
    }
    print_list (interests);
    return 0;
}
