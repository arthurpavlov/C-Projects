#include "mismatch_server.h"

/*
 * Socket code re-factored from https://www.cs.cmu.edu/afs/cs/academic/class/115213-f99/www/class26/tcpserver.c
 * */

int parent_fd; //Parent socket
int child_fd; //Child socket
int port_num = PORT; //Port to listen on
socklen_t  clientlen; //Byte size of client's address
struct sockaddr_in serveraddr; //Server's address
struct sockaddr_in clientaddr; //Client's address
struct hostent *hostp; //Client's host information
char server_buffer[BUFFER_SIZE]; //Dedicated buffer for calls that can't be made via client
char *hostaddrp; //Client's IP address
int optval; //Flag value for setsockopt function

QNode *root = NULL; //Root of question tree
Node * interests = NULL; //List of questions
Client * registered_users = NULL; //List of all registered clients
Node * connected_fds = NULL; //List of all file descriptors connected to server
Node * online_fds = NULL; //List of all file descriptors representing online users

/*
 * Shuts down server and cleans tree. Called by catch_sig().
 * */
void terminate_server(int code){
	free_tree(root);
	printf("\nServer has been shut down\n");
	exit(1);
}

/*
 * Catches SIGTSTP and SIGINT calls in Command Prompt. Then calls terminate_server().
 * */
int catch_sig(int sig, void (*handler)(int)) {
    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    return sigaction(sig, &action, NULL);
}

/*
 * Wrapper function for raising errors.
 * */
void error_handler(char *error_msg) {
  perror(error_msg);
  exit(1);
}

/*
 * Returns the Client object from registered_users with a file descriptor <user_fd>.
 * */
Client *get_registered_client_by_fd(int user_fd){
	Client *reg_user = registered_users;
	while(reg_user != NULL){
		if((reg_user->fd == user_fd)){
			return reg_user;
		}
		reg_user = reg_user->next;
	}
	return NULL;
}

/*
 * Returns the Client object from registered_users with the name <user_name>.
 * */
Client *get_registered_client_by_name(char *user_name){
	Client *reg_user = registered_users;
	while(reg_user != NULL){
		if((strcmp(reg_user->name, user_name) == 0)){
			return reg_user;
		}
		reg_user = reg_user->next;
	}
	return NULL;
}

/*
 * Writes to <message> to user with file descriptor <user_fd>.
 * Raises error if write() call fails.
 * */
void message_user(int user_fd, char *buffer, char *message){
	//copy into buffer
	snprintf(buffer, BUFFER_SIZE, "%s", message);
	int byte_size;
	byte_size = write(user_fd, buffer, BUFFER_SIZE);
	if (byte_size < 0){
		error_handler("ERROR writing to socket");
	}
	bzero(buffer, BUFFER_SIZE);
}

/*
 * Processes post command <cmd> and sends message from <user>.
 * */
int post_command_message(Client *user, char *cmd){
	char *target_name = malloc(BUFFER_SIZE);
	char user_message[BUFFER_SIZE];
	bzero(user_message, BUFFER_SIZE);
	int is_message_empty = 1;
	char *arg;
	int i = 1;
	arg = strtok(cmd, " ");
	while(arg != NULL ){
		//Extract target name
		if((i == 2)){
			strcpy(target_name, arg);
		}
		//Extract user message
		else if((i > 2)){
			if((is_message_empty)){
				strcpy(user_message, arg);
				is_message_empty = 0;
			}
			else{
				strcat(user_message, " ");
				strcat(user_message, arg);
			}
		}
		arg = strtok(NULL, " ");
		i++;
	}
	//Get target client
	Client *target_client = get_registered_client_by_name(target_name);
	//Send message to target client
	message_user(target_client->fd, target_client->buf, user->name);
	message_user(target_client->fd, target_client->buf, " sent you a message!\n");
	message_user(target_client->fd, target_client->buf, user_message);
	message_user(target_client->fd, target_client->buf, "\n");
	free(target_name);
	return 1;
}

/*
 * Helper function that writes greeting to user with file descriptor <user_fd>.
 * */
void print_greetings(int user_fd) {
	message_user(user_fd, server_buffer, "----------------------------------------------\n");
	message_user(user_fd, server_buffer, "Friend recommender system. Find people who are absolutely not like you!\n");
	message_user(user_fd, server_buffer, "CSC209 fall 2016 team. All rights reserved\n");
	message_user(user_fd, server_buffer, "----------------------------------------------\n");
}

/*
 * Helper function that writes the command select option to user with file descriptor <user_fd>.
 * */
void print_command_select_prompt(int user_fd){
	message_user(user_fd, server_buffer, "please select a command! [do_test, get_all, post <user> <message>, quit]\n");
}

/*
 * Helper function that writes the unique list of mismatches of <client> to <client>.
 * */
void print_mismatches_to_user(Client *client, Node * mismatch_list){
	while (mismatch_list != NULL){
		message_user(client->fd, client->buf, mismatch_list->str);
		message_user(client->fd, client->buf, "\n");
		mismatch_list = mismatch_list->next;
	}
}

/*
 * Processes user input from <file_d> and returns a string pointer converted from carriage return line feed format.
 * If read() reads 0 bytes of data, it signifies that user with file descriptor <file_d> has disconnect by pressing cmd+c,
 * in which case we manually overwrite the output string to "quit".
 * Raises error if read() fails.
 * */
char *convert_from_crlf(int file_d){
	char buffer_1[BUFFER_SIZE];
	bzero(buffer_1, BUFFER_SIZE);
	int bytes = read(file_d, buffer_1, BUFFER_SIZE);
	sleep(3);
	if (bytes < 0){
		error_handler("ERROR reading from socket");
	}
	//User quit via cmd c
	if((bytes == 0)){
		buffer_1[0] = 'q';
		buffer_1[1] = 'u';
		buffer_1[2] = 'i';
		buffer_1[3] = 't';
		buffer_1[4] = '\0';
	}
	//User gave input
	else{
		int where = find_network_newline(buffer_1, BUFFER_SIZE);
		if((where>=0)){
			buffer_1[where] = '\0';
			buffer_1[where+1] = '\0';
		}
	}
	char *after = &buffer_1[0];
	return after;
}

/*
 * Returns 1 if there exists a client in registered_users with name <user_name>.
 * Returns 0 if there doesn't exist a client in registered_users with name <user_name>.
 * */
int check_if_user_is_registered(char *user_name){
	Client *reg_user = registered_users;
	while(reg_user != NULL){
		if((strcmp(reg_user->name, user_name) == 0)){
			return 1;
		}
		reg_user = reg_user->next;
	}
	return 0;
}

/*
 * Adds the Client object <client> to registered_users.
 * */
void add_registered_user(Client *client){
	if((registered_users == NULL)){
		registered_users = client;
		client->next = NULL;
	}
	else{
		Client *reg_user = registered_users;
		while(reg_user->next != NULL){
			reg_user = reg_user->next;
		}
		reg_user->next = client;
		client->next = NULL;
	}
}

/*
 * If <dest> is 1, add_fd adds a new *Node with file descriptor <user_fd> and char* <name> to connected_fds.
 * If <dest> is 0, add_fd adds a new *Node with file descriptor <user_fd> and char* <name> to online_fds.
 * */
void add_fd(int user_fd, int dest, char *name){
	Node *new_fd = malloc(sizeof(Node)*1);
	new_fd->fd = user_fd;
	new_fd->str = name; //used when adding online fd
	new_fd->next = NULL;
	//Add to connected_fds
	if((dest)){
		if((connected_fds == NULL)){
			connected_fds = new_fd;
		}
		else{
			Node *con_fd = connected_fds;
			while(con_fd->next != NULL){
				con_fd = con_fd->next;
			}
			con_fd->next = new_fd;
			con_fd->next->next = NULL;
		}
	}
	//Add to online_fds
	else{
		if((online_fds == NULL)){
			online_fds = new_fd;
		}
		else{
			Node *on_fd = online_fds;
			while(on_fd->next != NULL){
				on_fd = on_fd->next;
			}
			on_fd->next = new_fd;
			on_fd->next->next = NULL;
		}
	}

}

/*
 * If <dest> is 1, remove_fd removes Node* with file descriptor <user_fd> from connected_fds.
 * If <dest> is 0, remove_fd removes Node* with file descriptor <user_fd> from online_fds.
 * */
void remove_fd(int user_fd, int from){
	//Remove from connected_fds
	if((from)){
		if(connected_fds->fd == user_fd){
			if((connected_fds->next == NULL)){
				connected_fds = NULL;
			}
			else{
				connected_fds = connected_fds->next;
			}
		}
		else{
			Node * pre_rmv_item = connected_fds;
			while(pre_rmv_item->next->fd != user_fd){
				pre_rmv_item = pre_rmv_item->next;
			}
			Node *rplc_node = pre_rmv_item->next->next;
			free(pre_rmv_item->next);
			pre_rmv_item->next = rplc_node;
		}
	}
	//Remove from online_fds
	else{
		if(online_fds->fd == user_fd){
			if((online_fds->next == NULL)){
				//free(online_fds->str);
				free(online_fds);
				online_fds = NULL;
			}
			else{
				online_fds = online_fds->next;
			}
		}
		else{
			Node * pre_rmv_item = online_fds;
			while(pre_rmv_item->next->fd != user_fd){
				pre_rmv_item = pre_rmv_item->next;
			}
			Node *rplc_node = pre_rmv_item->next->next;
			free(pre_rmv_item->next);
			pre_rmv_item->next = rplc_node;
		}
	}
}

/*
 * Extracts input from file descriptor <file_d> and processes the input with the help of check_username().
 * If the user input is a valid username, a valid oversized username, or "quit", process_username returns
 * the char* to that string.
 * If the user input is an invalid username, process_username returns NULL.
 * */
//char *username if found or quit NULL invalid
char *process_username(int file_d){
	char* username_input = malloc(BUFFER_SIZE);
	strcpy(username_input, convert_from_crlf(file_d));
	int check = check_username(username_input);
	//Valid username or Quit
	if((check == 1 || check == -1)){
		return username_input;
	}
	//Valid truncated username
	else if ((check == 2)){
		message_user(file_d, server_buffer, "Your username was truncated because it exceeded the 128 char limit\n");
		strncpy(username_input, username_input, MAX_NAME);
		username_input[MAX_NAME] = '\0';
		return username_input;
	}
	//Invalid username
	else{
		free(username_input);
		return NULL;
	}
}

/*
 * process_command evaluates user input with the help of check_command_format().
 * If the user input is quit, process_command returns -1.
 * If the user input is invalid, process_command returns 0.
 * If the user input is a valid call to do_test, process_command returns 1.
 * If the user input is a valid call to get_all, process_command returns 2.
 * If the user input is a valid call to post, process_command returns 3.
 * */
int process_command(Client *client){
	bzero(client->buf, BUFFER_SIZE);

	char *user_command = malloc(BUFFER_SIZE);
	//Get command input
	strcpy(user_command, convert_from_crlf(client->fd));
	//Create duplicate 1
	char command_copy_1[BUFFER_SIZE];
	bzero(command_copy_1, BUFFER_SIZE);
	strcpy(command_copy_1, user_command);
	//Create duplicate 2
	char command_copy_2[BUFFER_SIZE];
	bzero(command_copy_2, BUFFER_SIZE);
	strcpy(command_copy_2, user_command);
	//Get id of format checked command
	int command_id = check_command_format(command_copy_1);
	//User Quit
	if((command_id == 3)){
		free(user_command);
		return -1;
	}
	//Invalid Command
	else if((command_id == 0)){
		message_user(client->fd, client->buf, "Command Not Supported\n");
		free(user_command);
		return 0;
	}
	//do_test Command
	else if(command_id == 1){
		//Check if user has taken test
		if((client->taken_test == 0)){
			free(user_command);
			return 1;
		}
		else{
			message_user(client->fd, client->buf, "You have already completed the test!\n");
			free(user_command);
			return 0;
		}
	}
	//get_all Command
	else if((command_id == 2)){
		if((client->taken_test == 1)){
			free(user_command);
			return 2;
		}
		else{
			message_user(client->fd, client->buf, "You need to complete the test to run get_all\n");
			free(user_command);
			return 0;
		}
	}
	//post Command
	else{
		char * argument_parameter;
		argument_parameter = strtok(command_copy_2, " ");
		int parameter_index = 1;
		char user_message[2*BUFFER_SIZE];
		bzero(user_message, BUFFER_SIZE);
		int is_message_empty = 1;
		while (argument_parameter != NULL){
			//Skip post argument (already checked in check_command_format)
			if((parameter_index == 1)){
				argument_parameter = strtok(NULL, " ");
			}
			//Check that target user
			else if((parameter_index == 2)){
				//Check that target user is registered
				if((!check_if_user_is_registered(argument_parameter))){
					message_user(client->fd, client->buf, argument_parameter);
					message_user(client->fd, client->buf, " is not a registered username\n");
					free(user_command);
					return 0;
				}
				//Check that target user is online
				else if((!check_list_for_user(online_fds, argument_parameter))){
					message_user(client->fd, client->buf, "User ");
					message_user(client->fd, client->buf, argument_parameter);
					message_user(client->fd, client->buf, " is not online at the moment\n");
					free(user_command);
					return 0;
				}
				//Check that target user is not the user sending the message
				else if((strcmp(client->name, argument_parameter) == 0)){
					message_user(client->fd, client->buf, "You cannot send a message to yourself!\n");
					free(user_command);
					return 0;
				}
				else{
					argument_parameter = strtok(NULL, " ");
				}
			}
			//Check message
			else{
				//Check that message doesn't exceed 1024 chars
				if((strlen(user_message) > BUFFER_SIZE)){
					message_user(client->fd, client->buf, "message exceeds char limit\n");
					free(user_command);
					return 0;
				}
				else{
					if((is_message_empty)){
						strcpy(user_message, argument_parameter);
						is_message_empty = 0;
					}
					else{
						strcat(user_message, " ");
						strcat(user_message, argument_parameter);
					}
				}
				argument_parameter = strtok(NULL, " ");
			}
			parameter_index++;
		}
		//Valid post message
		post_command_message(client, user_command);
		free(user_command);
		return 3;
	}
}

/*
 * process_answer evaluates user input with the help of check_answer_format().
 * If user input is "quit", process_answer() returns -1.
 * If user input is an invalid statement, process_answer() returns 0.
 * If user input is valid yes or no statement, process_answer() returns 1.
 * */
int process_answer(Client *user){
	char *answer = malloc(sizeof(char)*200);
	char *user_input = convert_from_crlf(user->fd);
	strncpy(answer, user_input, strlen(user_input));
	answer[strlen(user_input)] = '\0';
	//Quit
	if((check_answer_format(answer) == -1)){
		free(answer);
		return -1;
	}
	else if((check_answer_format(answer) == 1)){
		//Yes
		if((strncmp(answer, "y", strlen("y")) == 0)){
			user->answers[user->questions_answered] = 1;
			user->questions_answered++;
			free(answer);
			return 1;
		}
		//No
		else if((strncmp(answer, "n", strlen("y")) == 0)){
			user->answers[user->questions_answered] = 0;
			user->questions_answered++;
			free(answer);
			return 1;
		}
	}
	free(answer);
	//Invalid answer
	return 0;
}

int main(int argc, char **argv) {

  if (argc < 2) {
     printf ("To run the program ./mismatch_server <name of input file>\n");
     return 1;
  }
  interests = get_list_from_file ( argv[1]);
  if ((interests == NULL)){
	 printf ("Input file is empty\n");
	 exit(0);
  }
  int number_of_questions;
  number_of_questions = get_question_count(interests);
  free_list(interests);	//Reset interests
  interests = get_list_from_file ( argv[1]); //Get interests
  root = add_next_level (root,  interests); //create tree
  free_list(interests);	//Reset interests
  interests = get_list_from_file ( argv[1]);
  parent_fd = socket(AF_INET, SOCK_STREAM, 0); //Create parent socket
  if ((parent_fd < 0)){
	  error_handler("ERROR opening socket");
  }
  add_fd(parent_fd, 1, "parent"); //Add parent fd to connect_fds
  optval = 1;
  //Allow for port release
  setsockopt(parent_fd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));
  if (optval == -1){
	  perror("setsockopt -- REUSEADDR");
  }
  bzero((char *) &serveraddr, sizeof(serveraddr)); //set up serveraddr
  serveraddr.sin_family = AF_INET; //Set address family
  serveraddr.sin_addr.s_addr = INADDR_ANY; //Set Interet Address
  serveraddr.sin_port = htons((unsigned short)port_num); //Set Port Number
  if (bind(parent_fd, (struct sockaddr *) &serveraddr,sizeof(serveraddr)) < 0) //Bind socket
	  error_handler("ERROR on binding");
  if (listen(parent_fd, 5) < 0) //Set socket to list and allow up to 5 requests to queue up
	  error_handler("ERROR on listen");
  printf("Listening on %d\n", port_num);
  clientlen = sizeof(clientaddr);
  catch_sig(SIGTSTP, terminate_server); //Set up signal handler for SIGTSTP Signal
  catch_sig(SIGINT, terminate_server); //Set up signal handler for SIGINT Signal

  int max_fd; //The fd with the largest value. Needed for select
  fd_set readfds; //Set of fds that are ready to be read
  int ready_files; //The number of fds ready to be read

  while (1) {
	  FD_ZERO(&readfds); //Clear read set
	  FD_SET(parent_fd, &readfds); //Allways add parent. Needed for new connections
	  max_fd = parent_fd;
	  //Fill with all fds in connected_fds
	  Node *con_fd = connected_fds;
	  while(con_fd != NULL){
		  if(con_fd->fd > 0) //Check if valid fd
			  FD_SET(con_fd->fd, &readfds);
		  if(con_fd->fd > max_fd) //Find fd with largest value
			  max_fd = con_fd->fd;
		  con_fd = con_fd->next;
	  }
	  //Select users that wrote something
	  ready_files = select(max_fd + 1, &readfds, NULL, NULL, NULL);
	  if (ready_files < 0)
		  error_handler("select error");
	  //Check if parent wrote something (check for connection request)
	  if (FD_ISSET(parent_fd, &readfds)){
		  //Add new connection
		  child_fd = accept(parent_fd, (struct sockaddr *)&clientaddr, &clientlen);
		  if (child_fd < 0)
			  error_handler("accept error");
		  hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,sizeof(clientaddr.sin_addr.s_addr), AF_INET);
		  if (hostp == NULL)
			  error_handler("ERROR on gethostbyaddr");
		  hostaddrp = inet_ntoa(clientaddr.sin_addr); //Get ip address
		  if (hostaddrp == NULL)
			  error_handler("ERROR on inet_ntoa\n");
		  printf("connection with %s\n", hostaddrp);
		  //Add to connected_fds
		  add_fd(child_fd, 1, "child");
		  print_greetings(child_fd);
		  message_user(child_fd, server_buffer, "Can i get your username?\n");
	  }
	  //Read from existing fd
	  else{
		  Node *con_fd_2 = connected_fds;
		  while(con_fd_2 != NULL){
			  //Check if fd wrote something
			  if (FD_ISSET(con_fd_2->fd , &readfds)){
				  //Not Online
				  if((!check_list_for_fd(online_fds, con_fd_2->fd))){
					  //Username Select
					  char *username_input = process_username(con_fd_2->fd);
					  //Invalid format or username is already being used by online user
					  if(username_input == NULL || check_list_for_user(online_fds, username_input)){
						  message_user(con_fd_2->fd, server_buffer, "Invalid username!\n");
						  message_user(con_fd_2->fd, server_buffer, "Can i get your username?\n");
						  free(username_input);
					  }
					  //User Quit
					  else if(is_quit(username_input)){
						  printf("lost connection to %s\n", get_ip_from_fd(con_fd_2->fd));
						  remove_fd(con_fd_2->fd, 1);
						  free(username_input);
						  close(con_fd_2->fd);
					  }
					  //Returning client
					  else if((check_if_user_is_registered(username_input))){
						  Client *returning_client = get_registered_client_by_name(username_input);
						  returning_client->fd = con_fd_2->fd;
						  add_fd(con_fd_2->fd, 0, returning_client->name);
						  free(username_input);
						  printf("%s has reconnected\n", returning_client->name);
						  print_command_select_prompt(con_fd_2->fd);
					  }
					  //New client
					  else{
						  Client *new_client = malloc(100*sizeof(Client));
						  new_client->fd = con_fd_2->fd;
						  bzero(new_client->name, MAX_NAME);
						  strcpy(new_client->name, username_input);
						  free(username_input);
						  new_client->my_questions = interests;
						  new_client->answers =  malloc(30 * sizeof(int));
						  new_client->questions_answered = 0;
						  new_client->taken_test = 0;
						  new_client->next = NULL;
						  new_client->status = 1; //the new client is in command select phase
						  add_fd(con_fd_2->fd, 0, new_client->name);
						  add_registered_user(new_client);
						  printf("%s has connected\n", new_client->name);
						  print_command_select_prompt(con_fd_2->fd);
					  }
				  }
				  //Online
				  else{
					  Client *online_user = get_registered_client_by_fd(con_fd_2->fd);
					  //In Command Select
					  if((online_user->status == 1)){
						  int command_id = process_command(online_user);
						  //Quit
						  if((command_id == -1)){
							  printf("Lost connection to %s\n", get_ip_from_fd(con_fd_2->fd));
							  printf("Removing client %s\n", online_user->name);
							  remove_fd(online_user->fd, 1);
							  remove_fd(online_user->fd, 0);
							  //Reference to fd that needs to be closed
							  int closing_val = online_user->fd;
							  online_user->fd = 10;
							  close(closing_val);
						  }
						  //Invalid Command
						  else if((command_id == 0)){
							  printf("%s entered an invalid command\n", online_user->name);
							  print_command_select_prompt(con_fd_2->fd);
						  }
						  //Valid do_test Command
						  else if((command_id == 1)){
							  printf("%s executing do_test\n", online_user->name);
							  online_user->status = 2;
							  message_user(online_user->fd, online_user->buf, "Do you like ");
							  message_user(online_user->fd, online_user->buf, online_user->my_questions->str);
							  message_user(online_user->fd, online_user->buf, " ?(y/n)\n");
							  online_user->my_questions = online_user->my_questions->next;
						  }
						  //Valid get_all Command (process_command checks if test complete)
						  else if((command_id == 2)){
						  	  printf("%s executing get_all\n", online_user->name);
						  	  Node * list_with_user = search_user(root, online_user->name);
						  	  //Get user tree node
						  	  Node * user_node = get_user(list_with_user, online_user->name);
						  	  //Get list of Mismatches
						  	  Node * mismatches = get_mismatch(root, user_node->useranswers, number_of_questions);
						  	  //No Mismatches
						  	  if((mismatches == NULL)){
						  		  message_user(online_user->fd, online_user->buf, "\nYou have no matches! Try Again Later!\n\n");
						  		  print_command_select_prompt(con_fd_2->fd);
						  	  }
						  	  //Existing Mismatches
						  	  else{
						  		  message_user(online_user->fd, online_user->buf, "\nYou have been matched with...\n");
						  		  print_mismatches_to_user(online_user, mismatches);
						  		  message_user(online_user->fd, online_user->buf, "\n");
						  		  print_command_select_prompt(con_fd_2->fd);
						      }
						  }
						  //Valid post Command
						  else{
						  	  printf("%s executing post\n", online_user->name);
						  	  message_user(online_user->fd, online_user->buf, "Message has been sent!\n");
						  	  print_command_select_prompt(con_fd_2->fd);
						  }
					  }
					  //User is in the process of taking the test
					  else{
						  //Check answer
						  int user_answer = process_answer(online_user);
						  //Invalid
						  if((user_answer == 0)){
							  message_user(online_user->fd, online_user->buf, "Invalid answer!\n");
						  }
						  //Valid
						  else if((user_answer == 1)){
							  //User Completed test
							  if((online_user->questions_answered == number_of_questions)){
								  online_user->taken_test = 1;
								  //Put user back into Command Select
								  online_user->status = 1;
								  add_user(root, online_user->answers, online_user->name, online_user->answers);
								  message_user(online_user->fd, online_user->buf, "Test Complete!\n");
								  print_command_select_prompt(con_fd_2->fd);
							  }
							  //User still taking test
							  else{
								  //Ask next question
								  message_user(online_user->fd, online_user->buf, "Do you like ");
								  message_user(online_user->fd, online_user->buf, online_user->my_questions->str);
								  message_user(online_user->fd, online_user->buf, " ?(y/n)\n");
								  online_user->my_questions = online_user->my_questions->next;
							  }

						  }
						  //User quit while taking test (reset questions, answers, count)
						  else{
							  //reset user and put back into Command Select
							  online_user->questions_answered = 0;
							  online_user->my_questions = interests;
							  online_user->status = 1;
							  printf("Lost connection to %s\n", get_ip_from_fd(con_fd_2->fd));
							  printf("Removing client %s\n", online_user->name);
							  remove_fd(online_user->fd, 1);
							  remove_fd(online_user->fd, 0);
							  int closing_val = online_user->fd;
							  //Change user's file descriptor to a place holder 20
							  online_user->fd = 20;
							  close(closing_val);
						  }
					  }
				  }
			  }
		  //Cycle to next file descriptor
		  con_fd_2 = con_fd_2->next;
		  }
	  }
   }
}









