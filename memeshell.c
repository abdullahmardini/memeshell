/*
 * Wanted to try to write a simple shell again. My goal is use cowsay to push
 * everything you type through a cow, or some other art. The plan is literally
 * to be ridiculous.
 * I also wanted a fairly usable shell. Right now, my main goal is to support
 * git, so that I can actually stuff in this shell. Then I should support piping
 * and input/output direction, but that may be a pipe dream :p.
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
#define LINE_BUFFER 	512
#define ARG_MAX			15
#define ARG_BUF			256
#define CONTINUE		1
#define STOP			0


/* Stealing control+c so you can't exit the program that way. >:) */
void sigint_handler(int signum);
/* Prints the shell stuff. Eventually going to do more. */
void print_shell(void);
/* Reads the input a line at a time. Using fgets. We use the status of fgets to
 * determine if we've hit an EOF and should quit. */
char *read_line(void);
/* Parses the line to get the commands and the arguments. Uses strtok because
 * it's more elegant and less error prone than reading char by char. */
char **read_args(char *line);
/* Takes the parsed line, and runs the command it got from it. */
int run_cmd(char **cmd);
/* Built in pwd tool. */
int pwd();
/* Uses chdir to change the directory. TODO: cd with nothing should cd to your
 * home directory. */
int cd(char **cmd);
/* Uses fork and execvp to call an external application. */
int external(char **cmd);
/* Delimiters I use to know when to stop parsing a given character. */
int line_delim(char c);


int main(void) {
	//signal(SIGINT, sigint_handler);
	char *line, **cmd;
	int status = CONTINUE;

	while(status) {
		print_shell();
		line = read_line();
		cmd = read_args(line);
		status = run_cmd(cmd);

		free(line);
		for (int i = 0; cmd[i] != NULL; i++)
			free(cmd[i]);
		free(cmd);
	}
	printf("You'll miss me... \n");
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

char **read_args(char *line){
	char **cmd = malloc(sizeof(char*) * ARG_MAX), *token;

	if (cmd == NULL) {
		fprintf(stderr, "Your system has some issues.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0, j = 0, k = 0; i < LINE_BUFFER; j++, k = 0) {
		/* Stop when we hit a new line character. Yes I know bash can do some
		 * advanced magic with \, but this isn't bash darn it.
		 * Otherwise, we'll allocate some space and do stuff. */
		if (line[i] == '\n') {
			break;
		} else if (cmd[j] == NULL) {
			cmd[j] = malloc(sizeof(char) * ARG_BUF);
			if (cmd[j] == NULL) {
				fprintf(stderr, "Your system has some issues.\n");
				exit(EXIT_FAILURE);
			}
			memset(cmd[j], 0, ARG_BUF);
		} else {
			/* Either way, we should clear that buffer. */
			memset(cmd[j], 0, ARG_BUF);
		}

		/* Eat up all the white space.*/
		while (line[i] == ' ') {
			i++;
		}

		/* I want to treat anything in a paranthesis as verbatim. Unfortunately,
		 * strtok did not allow this level of granularity. I wish it did :/ . */
		if (line[i] == '"') {
			do {
				cmd[j][k] = line[i];
				i++;
				k++;
			} while (line[i] != '"');
			/* Copy over that trailing close paranthesis. */
			cmd[j][k] = line[i];
			i++;
			k++;
		} else {
			/* Otherwise, I'll just copy over what I see in the line. */
			while (line_delim(line[i])) {
				cmd[j][k] = line[i];
				i++;
				k++;
			}
		}
	}

	return cmd;
}

int run_cmd(char **cmd) {
	if (cmd[0][0] == '\0') {
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
	if (cmd[1] == NULL) {
		if(chdir("~") != 0) {
			fprintf(stderr, "How?!?!?!?\n");
		}
	} else if (cmd[2] != NULL) {
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

int line_delim(char c) {
	if (c == ' ' || c == '\n' || c == '\t' || c == '\a')
		return STOP;
	else
		return CONTINUE;
}