/*
 * Wanted to try to write a simple shell again. Ideally, I want to bundle this
 * with cowsay, for kicks and giggles.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

/*
 * Making some macros to improve readability and portability.
 */
#define LINE_BUFFER  512
#define ARG_MAX      8
#define ARG_BUF      64

#define TOKEN_DELIM " \t\r\n\a"

#define CONTINUE     1
#define STOP         0

/* Stealing control+c so you can't exit the program that way. */
void sigint_handler(int signum);
/* Prints the shell stuff. Eventually going to do more. */
void print_shell(void);
/* Reads the input a line at a time. */
char *read_line(void);
/* Parses the line to get the commands and the arguments. */
char **read_args(char *line);
/* Checks if we've reach a character I should treat as a delimiter. */
int line_delim(char c);
/* Takes the parsed line, and runs the command it got from it. */
int run_cmd(char **cmd);
/* Built in pwd tool. */
int pwd();
/* chdir to change the directory. */
int cd(char **cmd);
/* Uses fork and execvp to call an external tool. */
int external(char **cmd);

int main(void) {
	signal(SIGINT, sigint_handler);
	char *line, **cmd;
	int status = CONTINUE;

	while(status) {
		print_shell();
		line = read_line();
		cmd = read_args(line);
		status = run_cmd(cmd);

		free(line);
		//for (int i = 0; cmd[i] != NULL; i++)
		//	free(cmd[i]);
		free(cmd);
	}
	printf("\n");
	return EXIT_SUCCESS;
}

void print_shell() {
		/* Print prompt */
		printf("memeshell> ");
		fflush(stdout);
}

char *read_line(void){
	char *line = malloc(sizeof(char) * LINE_BUFFER);
	if (line == NULL) {
		fprintf(stderr, "Your system has some issues.\n");
		exit(EXIT_FAILURE);
	}
	memset(line, 0, LINE_BUFFER);
	if (fgets(line, LINE_BUFFER, stdin) == NULL)
		exit(EXIT_SUCCESS);
	return line;
}

int line_delim(char c){
	if (c == ' ' || c == '\0' || c == '\n')
		return 1;
	return 0;
}

char **read_args(char *line){

	// /* Initialize and clear the argument buffer. */
	// char **cmd = malloc(sizeof(char*) * ARG_MAX);
	// if (cmd == NULL) {
	// 	fprintf(stderr, "Your system has some issues.\n");
	// 	exit(EXIT_FAILURE);
	// }

	// // for (int i = 0; i < ARG_MAX; i++) {
	// // 	cmd[i] = malloc(sizeof(char) * ARG_BUF);
	// // 	if (cmd[i] == NULL) {
	// // 		fprintf(stderr, "Your system has some issues.\n");
	// // 		exit(EXIT_FAILURE);
	// // 	}
	// // 	memset(cmd[i], 0, ARG_BUF);
	// // }

	// if (line[0] == '\n') {
	// 	cmd[0][0] = '\n';
	// 	return cmd;
	// }

	// int i = 0, j = 0, k = 0;

	// /* Trim any whitespace in the beginning. */
	// while (line_delim(line[i])) {
	// 	i++;
	// }

	// for (; i < LINE_BUFFER, j < ARG_MAX; i++) {
	// 	if (cmd[j] == NULL) {
	// 		cmd[j] = malloc(sizeof(char) * ARG_BUF);
	// 		if (cmd[j] == NULL) {
	// 			fprintf(stderr, "Your system has some issues.\n");
	// 			exit(EXIT_FAILURE);
	// 		}
	// 		memset(cmd[j], 0, ARG_BUF);
	// 	}
	// 	if (line_delim(line[i])) {
	// 		/* Null terminate the string. */
	// 		//cmd[j][k] = '\0';
	// 		j++;
	// 		k = 0;
	// 	} else {
	// 		cmd[j][k] = line[i];
	// 		k++;
	// 	}
	// }
	char **cmd = malloc(sizeof(char*) * ARG_MAX), *token;
	int i;
	if (cmd == NULL) {
		fprintf(stderr, "Your system has some issues.\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, TOKEN_DELIM);

	for (i = 0; i < ARG_MAX - 1 || token != NULL; i++) {
		cmd[i] = token;
		token = strtok(NULL, TOKEN_DELIM);
	}

	cmd[++i] = NULL;

	return cmd;
}

int run_cmd(char **cmd) {
	if (cmd[0]== NULL) {
		return CONTINUE;
	} else if (strcmp(cmd[0], "exit") == 0) {
		return STOP;
	} else if (strcmp(cmd[0], "cd")  == 0) {
		return cd(cmd);
	} else if (strcmp(cmd[0], "pwd")  == 0) {
		return pwd();
	} else {
		return(external(cmd));
	}

}

int pwd() {
	char curdir[LINE_BUFFER];
	getcwd(curdir, LINE_BUFFER);
	printf(" %s\n", curdir);
	return CONTINUE;
}

int cd(char **cmd){
	if (cmd[1][0] == '\0') {
		if(chdir("~") != 0) {
			fprintf(stderr, "How?!?!?!?\n");
		}
	} else if (cmd[2][0] != '\0') {
		fprintf(stderr, "Too many arguments. What even are you thinking? You can't be in two directory's at once. Seriously... Calm down man.\n");
	} else if (chdir(cmd[1]) != 0) {
		fprintf(stderr, "Not a directory or something.\n");
	}
	return CONTINUE;
}

void sigint_handler(int signum) {
	printf("BWA HA HA HA HA HA HA\n");
}

int external(char **cmd) {
	pid_t pid;
	int status;

	pid = fork();
	if (pid == -1)  {
		fprintf(stderr, "Error: fork failed.\n");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		execvp(cmd[0],cmd);
		fprintf(stderr, "Error: that's not a thing guy.\n");
		exit(EXIT_FAILURE);
	} else {
		waitpid(-1, &status, 0);
		fprintf(stderr,"%c\n", WEXITSTATUS(status));
		return CONTINUE;
	}
}