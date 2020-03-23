/*
 * Wanted to try to write a simple shell again. Ideally, I want to bundle this
 * with cowsay, for kicks and giggles.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Making some macros to improve readability and portability.
 */
#define LINE_BUFFER  512
#define ARG_MAX      8
#define ARG_BUF      64

#define CONTINUE     1
#define STOP         0

void print_shell(void);
char *read_line(void);
char **read_args(char *line);
int line_delim(char c);
int run_cmd(char **cmd);
int pwd();
int cd(char **cmd);

int main(void) {
	char *line, **args;
	int status = CONTINUE;

	while(status) {
		print_shell();
		line = read_line();
		args = read_args(line);
		status = run_cmd(args);

		free(line);
		for (int i = 0; i < ARG_MAX; i++)
			free(args[i]);
		free(args);
	}
	return EXIT_SUCCESS;
}

void print_shell() {
		/* Print prompt */
		printf("sshell$ ");
			fflush(stdout);
}

char *read_line(void){
	char *line = malloc(sizeof(char) * LINE_BUFFER);
	if (line == NULL) {
		fprintf(stderr, "Your system has some issues.\n");
		exit(EXIT_FAILURE);
	}
	memset(line, 0, LINE_BUFFER);
	fgets(line, LINE_BUFFER, stdin);
	return line;
}

int line_delim(char c){
	if (c == ' ' || c == '\0' || c == '\n')
		return 1;
	return 0;
}

char **read_args(char *line){
	char **args = malloc(sizeof(char*) * ARG_MAX);
	if (args == NULL) {
		fprintf(stderr, "Your system has some issues.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < ARG_MAX; i++) {
		args[i] = malloc(sizeof(char) * ARG_BUF);
		if (args[i] == NULL) {
			fprintf(stderr, "Your system has some issues.\n");
			exit(EXIT_FAILURE);
		}
		memset(args[i], 0, ARG_BUF);
	}

	int i = 0, j = 0, k = 0;

	/* Trim any whitespace in the beginning. */
	while (line_delim(line[i])) {
		i++;
	}

	for (; i < LINE_BUFFER; i++) {
		if (line_delim(line[i])) {
			j++;
			k = 0;
		} else {
			args[j][k] = line[i];
			k++;
		}
	}

	return args;
}

int run_cmd(char **cmd) {
	if (strcmp(cmd[0], "exit") == 0) {
		return STOP;
	} else if (strcmp(cmd[0], "cd")  == 0) {
		return cd(cmd);
	} else if (strcmp(cmd[0], "pwd")  == 0) {
		return pwd();
	} else {
	pid_t pid;
	int status;

	pid = fork();
	if (pid == -1)  {
		fprintf(stderr, "Error: fork failed.");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		execvp(cmd[0],cmd);
		fprintf(stderr, "Error: command not found\n");
		exit(EXIT_FAILURE);
	} else {
		waitpid(-1, &status, 0);
		return WEXITSTATUS(status);
		}
	}
}

int pwd()  {
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