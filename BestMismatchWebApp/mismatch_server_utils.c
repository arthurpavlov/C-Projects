#include "mismatch_server_utils.h"

/*
 * Helper function for process_answer(). 
 * Checks the format of input string <answer>.
 * If length of <answer> is larger than 4 or the first value doesn't equal to 'y' or 'n',
 * check_answer_fromat() returns 0.
 * If <answer> is "quit", check_answer_fromat() returns -1.
 * If <answer> is at most 3 values and the first values are either 'y' or 'n', then
 * check_answer_fromat() returns 1.
 * */
int check_answer_format(char *answer){
	int answer_len = strlen(answer);
	//Invalid answer
	if((answer_len > 4)){
		return 0;
	}
	//Quit
	else if((answer_len == 4 && strncmp(answer, "quit", strlen("quit")) == 0)){
		return -1;
	}
	//Check Yes/No
	if((answer_len < 4)){
		if((strncmp(answer, "y", strlen("q")) == 0 || strncmp(answer, "n", strlen("q")) == 0)){
			return 1;
		}
		return 0;
	}
	//Invalid
	return 0;
}

/*
 * Helper function for convert_from_crlf().
 * Returns the index of '\r' where the value after '\r' is '\n' from string <buffer>.
 * Returns -1 if "\r\n" is not a substring of <buffer>.
 * */
int find_network_newline(char *buffer, int in_buf){
	int buf_index;
	for(buf_index = 0; buf_index < in_buf - 1; buf_index++){
		if((buffer[buf_index] == '\r') && (buffer[buf_index + 1] == '\n')){
			return buf_index;
		}
	}
	return -1;
}

/*
 * Helper function for process_command().
 * Checks the format of <user_cmd>
 * If <user_cmd> has an invalid command format, check_command_format() returns 0.
 * If <user_cmd> has a valid do_test command format, check_command_format() returns 1.
 * If <user_cmd> has a valid get_all command format, check_command_format() returns 2.
 * If <user_cmd> is "quit", check_command_format() returns 3.
 * If <user_cmd> has a valid post command format, check_command_format() returns 4.
 * */
int check_command_format(char *user_cmd){
	int arg_count = 0;
	size_t cmd_len = strlen(user_cmd);
	char *arg;
	arg = strtok(user_cmd, " ");
	//Get the number of arguments
	while(arg != NULL){
		arg_count++;
		arg = strtok(NULL, " ");
	}
	//Ivalid Command
	if((cmd_len == 0)){
		return 0;
	}
	//1 arg input case, check(do_test, yes/no, get_all, quit)
	else if((arg_count == 1)){
		//Check for do_test command
		if((cmd_len == 7 && strncmp(user_cmd, "do_test", strlen("do_test")) == 0)){
			return 1;
		}
		//Check for get_all command
		else if((cmd_len == 7 && strncmp(user_cmd, "get_all", strlen("get_all")) == 0)){
			return 2;
		}
		//Check for quit command
		else if((cmd_len == 4 && strncmp(user_cmd, "quit", strlen("quit")) == 0)){
			return 3;
		}
		//Invalid argument
		else{
			return 0;
		}
	}
	//Invalid argument
	else if((arg_count == 2)){
		return 0;
	}
	//3 arg input case, check post
	else {
		char * arg;
		arg = strtok(user_cmd, " ");
		if((strcmp(arg, "post") != 0)){
			return 0;
		}
		return 4;
	}
	return 1;
}

/*
 * Helper function that check if the string "quit" is equal to the string <buffer>.
 * If so, is_quit() returns 1.
 * If "quit" and string <buffer> don't equal, is_quit() returns 0.
 * */
int is_quit(char * buffer){
	if((strlen(buffer) == 4 && strncmp(buffer, "quit", strlen("quit")) == 0)){
		return 1;
	}
	return 0;
}

/*
 * Helper function for check_username.
 * Returns 1 if all value in <username> are either a letter or a number.
 * Returns 0 if there exists a single value in <username> that's not a letter or number.
 * */
int check_alphanum(char *username){
	int answer = 1;
	int char_index;
	char *name_char = &username[0];
	int usernamelen = strlen(username);
	for(char_index=0; char_index< usernamelen; char_index++){
		//Invalid char
		if((isalnum(*name_char)== 0)){
			answer = 0;
		}
		name_char++;
	}
	return answer;
}

//returns 0 if username is invalid, 1 if valid, 2 if valid but truncated, -1 quit
/*
 * Helper function for process_username.
 * If <username> is equal to "quit", check_username() returns -1.
 * If <username> has less than 8 characters or contains a value that not a letter or number,
 * check_username() returns 0.
 * If <username> exceeds the limit of a username length, check_username() returns 2.
 * If <username> is valid, check_username() returns 1.
 * */
int check_username(char *username){
	int username_len = strlen(username);
	if((username_len == 4 && strncmp(username, "quit", strlen("quit")) == 0)){
		return -1;
	}
	else if((check_alphanum(username) == 0 || username_len < 8)){
		return 0;
	}
	else if((username_len > 128)){
		return 2;
	}
	else{
		return 1;
	}
}

/*
 * Helper function for main();
 * Returns the IP Address of file descripted <input_fd>.
 * */
char *get_ip_from_fd(int input_fd){
	struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(input_fd, (struct sockaddr *)&addr, &addr_size);
    char client_ip[20];
    bzero(client_ip, 20);
    strcpy(client_ip, inet_ntoa(addr.sin_addr));
    char *client_ip_pntr = &client_ip[0];
    return client_ip_pntr;
}
