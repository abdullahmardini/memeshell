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


void print_shell(void);
char *read_line(void);
char **read_args(char *line);
int run_cmd(char **cmd);
int pwd();

int main(void) {
	char *line, **args;
	int status;

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
	fgets(line, LINE_BUFFER, stdin);
	return line;
}

char **read_args(char *line){
	char **args = malloc(sizeof(char*) * ARG_MAX);
	for (int i = 0; i < ARG_MAX; i++)
		args[i] = malloc(sizeof(char) * ARG_BUF);

	int i = 0, j = 0, k = 0;

	/* Trim any whitespace in the beginning. */
	while (line[i] == ' ') {
		i++;
	}

	for (; i < LINE_BUFFER; i++) {
		if (line[i] == ' ') {
			j++;
			k = 0;
			i++;
		}
		args[j][k] = line[i];
		k++;
	}

	return args;
}

int run_cmd(char **cmd) {
	if (strcmp(cmd[0], "exit") == 0) {
		return 0;
	} else if (strcmp(cmd[0], "cd")  == 0) {
		return chdir(cmd[0]);
	} else if (strcmp(cmd[0], "pwd")  == 0) {
		return pwd();
	} else {
	pid_t pid;
	int status;

	pid = fork();
	if (pid == -1)  {
		fprintf(stderr, "Error: fork failed.");
		exit(1);
	} else if (pid == 0) {
		execvp(cmd[0],cmd);
		fprintf(stderr, "Error: command not found\n");
		exit(1);
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
	return 0; //pwd literally can't fail
}