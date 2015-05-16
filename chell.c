/* Chell is a homage to the protagonist in the portal games. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define INP_LEN 256
#define CMD_LEN 256
#define ARG_LEN 256

#define BLACK   "\x1b[90m"
#define RED     "\x1b[91m"
#define GREEN   "\x1b[92m"
#define YELLOW  "\x1b[93m"
#define BLUE    "\x1b[94m"
#define MAGENTA "\x1b[95m"
#define CYAN    "\x1b[96m"
#define RESET   "\x1b[0m"

#define NAME "chell"

/* The 5 is which color index 0-7; then 0-255m.*/
/* Hex color codes can be calculated with: COLOR = r*6^2 + g*6 + b) + 16. */
#define SPECIAL "\x1b[38;5;150m"

const char * prompt = "%s(^._.^)ﾉ%s %s@%s %s%s%s %s$%s ";

int snprintf (char * s, size_t n, const char * format, ...);
void cd(char *);
void pwd(char *, size_t);
int isspace(int);
void exit(int);
int parse(char *, char **);
void checkEnv();
void execute(char **);
void print_prompt(char *, size_t);

int main(int argc, char const *argv[]) {
	/* Whole input string. */
	char input[INP_LEN] = "";
	
	char *args[32]; 
	int nwords;

	/* Working directory. */
	char wd[256];

	memset(wd, 0, 256);

	while (1) {
		print_prompt(wd, sizeof(wd));

		/* Get user input. */
		fgets(input, sizeof(input), stdin);

		nwords = parse(input, args);
		args[nwords] = NULL;

		/* a built-in command "exit" which terminates all remaining processes
		started from the shell in an orderly manner before exiting the shell
		itself */		
		if (nwords == 0) {
			continue;
		}
		if (strcmp("exit", args[0]) == 0) {
			exit(0);
		} else if (strcmp("cd", args[0]) == 0) {
			cd(args[1]);			
		} else if (strcmp("pwd", args[0]) == 0) {
			pwd(wd, sizeof(wd));
		} else if (strcmp("checkEnv", args[0]) == 0) {
			checkEnv();
		} else {
			execute(args);
		}

		/* Zero the strings. */
		memset(input, 0, strlen(input));
		/*memset(wd, 0, strlen(wd));*/
		/*free(words);*/
	}
	return 0;
}

int parse(char *line, char **argv) {
	/* Number of args. */
	int argc = 0;

	/* Ignore empty strings. */    
	if (strlen(line) == 1) {
		return argc;
	}

    /* Parse the whole line. */ 
    while (*line != '\0') {
		/* Replace whitespace with null-byte. */
		while (*line == ' ' ||
		       *line == '\n') {
			argc++;
			*line++ = '\0';
		}

		/* Save the argument. */
		*argv++ = line;          

		/* Skip until we need to remove whitespace.  */
		while (*line != '\0' &&
			   *line != ' '  &&
			   *line != '\n') {
			line++;
		}
    }
	return argc;
}

/* Doesn't work for ~root */
void cd(char * input) {
	int ret = 0;
	char path[256] = "";

	/* No argument -> go to home directory. */
	if (input == NULL) {
		ret = chdir(getenv("HOME"));
	} else if ('~' == input[0]) {
		strcat(path, "/home/");
		strcat(path, input+1);
		ret = chdir(path);
	} else {
		ret = chdir(input);
	}
	switch (ret) {
		case -1:
			fprintf(stderr, "cd: The directory “%s” does not exist\n", input);
	}
}

void pwd(char *wd, size_t size) {
	if (getcwd(wd, size) != NULL) {
		printf("Current working dir: %s\n", wd);
	} else {
		fprintf(stderr, "getcwd: %s\n", strerror(errno));
	}
}

/* Still broken, needs to be implemented with pipe. */
void checkEnv() {
	int ret = 0;
	char argument[ARG_LEN] = "";
	char *pager = getenv("PAGER");

	if (pager == NULL) {
		pager = "lessq";	
	}
	
	printf("pager: %s\n", pager);

	/* Necessary to check for errors? */
	snprintf(argument, ARG_LEN, "printenv | sort | %s", pager);
	printf("arg: %s\n", argument);
	
	ret = system(argument);
	printf("ret: %d\n", ret);
	if (ret != 0) {
		pager = "more";
	}
	
	snprintf(argument, ARG_LEN, "printenv | sort | %s", pager);
	system(argument);
}

void execute(char **argv) {
	pid_t pid;
	int status;

	/* Fork a child process. */
    if ((pid = fork()) < 0) {     
		fprintf(stderr, "*** ERROR: forking child process failed\n");
		exit(1);
    } else if (pid == 0) {
    	/* For the child process. */

    	/* Execute the command. */   
		if (execvp(*argv, argv) < 0) {
			fprintf(stderr, "%s: Unknown command: %s\n", NAME, argv[0]);
			exit(1);
		}
	} else {
		/* For the parent. */

    	/* Wait for completion. */
    	while (wait(&status) != pid);
	}
}

void print_prompt(char *wd, size_t size) {
	/* Fix prompt. */
	if (getcwd(wd, size) == NULL) {
		fprintf(stderr, "getcwd: %s\n", strerror(errno));
	}

	/* Print prompt. */
	printf(prompt, YELLOW, RESET, BLACK,
	RESET, MAGENTA, wd, RESET,
	BLUE, RESET);
}
