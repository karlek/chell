/* Chell is a homage to the protagonist in the portal games. */

/* Needed for sigrelse, sighold, snprintf and kill. */
#define _XOPEN_SOURCE 500

#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define INP_LEN 256
#define CMD_LEN 256
#define ARG_LEN 256

#define BLACK   "\x1b[90m"
#define RED		"\x1b[91m"
#define GREEN   "\x1b[92m"
#define YELLOW  "\x1b[93m"
#define BLUE	"\x1b[94m"
#define MAGENTA "\x1b[95m"
#define CYAN	"\x1b[96m"
#define RESET   "\x1b[0m"

#define NAME "chell"

#define WRITE 1
#define READ 0

/* The 5 is which color index 0-7; then 0-255m.*/
/* Hex color codes can be calculated with: COLOR = r*6^2 + g*6 + b) + 16. */
#define SPECIAL "\x1b[38;5;150m"

const char * prompt = "%s(^._.^)ﾉ%s %s@%s %s%s%s %s$%s ";

void background(int, char **);
void cd(char *);
void pwd(char *, size_t);
int parse(char *, char **, size_t);
void checkEnv();
void execute(char **);
void print_prompt(char *, size_t);

int main(int argc, char const *argv[]) {
	/* Whole input string. */
	char input[INP_LEN] = "";

	char *args[32];

	/* Working directory. */
	char wd[256];
	struct timeval begin;
	struct timeval end;

	int nwords = 0;

	long elapsed;

	memset(wd, 0, sizeof(wd));

	while (1) {
		print_prompt(wd, sizeof(wd));

		/* Get user input. */
		fgets(input, sizeof(input), stdin);

		/* Get string tokens. */
		nwords = parse(input, args, 32);
		switch (nwords) {
		case -1:
			fprintf(stderr, "chell: Too many arguments.\n");
		case 0:
			continue;
		}

		/* A built-in command "exit" which terminates all remaining processes
		started from the shell in an orderly manner before exiting the shell
		itself. */
		if (strcmp("exit", args[0]) == 0) {
			exit(0);
		} else if (strcmp("cd", args[0]) == 0) {
			cd(args[1]);
		} else if (strcmp("pwd", args[0]) == 0) {
			pwd(wd, sizeof(wd));
		} else if (strcmp("checkEnv", args[0]) == 0) {
			checkEnv(nwords, args);
		} else if (strcmp("&", args[nwords-1]) == 0) {
			/* Remove the '&'. */
			background(nwords, args);
		} else {
			gettimeofday(&begin, 0);
			execute(args);
			gettimeofday(&end, 0);
			elapsed = (end.tv_sec-begin.tv_sec)*1000000 + end.tv_usec-begin.tv_usec;
			printf("time: %.3fs\n", (double)elapsed/1000000);
		}

		/* Zero the strings. */
		memset(input, 0, strlen(input));
		/*free(words);*/
	}
	return 0;
}

void handler(int signum) {
	int status;
	wait(&status);
}

void background(int argc, char **argv) {
	pid_t pid;
	char command[256] = "";
	int i;

	/* Create command string. */
	for (i = 0; i < argc; i++) {
		strcat(command, argv[i]);
		if (i+1 == argc) {
			break;
		}
		strcat(command, " ");
	}
	argv[argc-1] = NULL;

	if((pid = fork()) == 0)
	{
		signal(SIGCHLD, handler);
		sighold(SIGCHLD);
		execute(argv);
		fprintf(stdout,"\n“%s” has ended.\n", command);
		sigrelse(SIGCHLD);
		exit(0);
	}
	kill(pid, SIGCHLD);
}

/* `ls ` is erroneous. */
int parse(char *line, char *argv[32], size_t size) {
	/* Number of args. */
	int argc = 0;
	char **tmp = argv;

	/* Ignore empty strings. */
	if (strlen(line) == 1) {
		return argc;
	}

	/* Parse the whole line. */
	while (*line != '\0') {
		if (argc > size) {
			return -1;
		}
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
	tmp[argc] = NULL;
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

char *get_pager() {
	pid_t pid_less;
	int status;

	char *which_args[] = {"which", "placeholder", NULL};
	char *pager = getenv("PAGER");

	int devNull, dup2Result;

	if (pager == NULL) {
		pager = "less";
	}

	which_args[1] = pager;

	if ((pid_less = fork()) == 0){
		devNull = open("/dev/null", O_WRONLY);
		if(devNull == -1){
			fprintf(stderr,"Error in open('/dev/null',0)\n");
			exit(EXIT_FAILURE);
		}
		dup2Result = dup2(devNull, STDOUT_FILENO);
		if(dup2Result == -1) {
		    fprintf(stderr,"Error in dup2(devNull, STDOUT_FILENO)\n");
		    exit(EXIT_FAILURE);
		}

		if (-1 == execvp(*which_args, which_args)) {
			exit(1);
		}
	}
	while (wait(&status) != pid_less);
	if (status == 256) {
		pager = "more";
	}
	return pager;
}

/* Still broken, needs to be implemented with pipe. */
void checkEnv(int argc, char **grep_args) {
/*	int status;
	int i;
*/
/*	char *printenv_args[] = {"printenv", NULL};
	char *sort_args[] = {"sort", NULL};
*/	char *pager_args[] = {"placeholder", NULL};

	/* Piping stuff. */
	pid_t pidPrintenv, pidSort;
	int status, pipa[2];

	char *pager = get_pager();
	pager_args[0] = pager;
	grep_args[0] = "grep";

	if(-1 == pipe(pipa)) {
		printf("\n\n\n\n\nerror!\n\n\n\n\n");
	}

	printf("args: %s\n", grep_args[1]);
	printf("pager: %s\n", pager);

	if((pidPrintenv = fork()) == 0) {
		dup2(pipa[WRITE], WRITE);
		close(pipa[READ]);
		close(pipa[WRITE]);
		execlp("printenv", "printenv", NULL);
	}

	if((pidSort = fork()) == 0)	{
		dup2(pipa[READ], READ);
		close(pipa[READ]);
		close(pipa[WRITE]);
		execlp("sort", "sort", NULL);
	}

	close(pipa[WRITE]);
	close(pipa[READ]);
	wait(&status);
	wait(&status);
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
	printf(
		prompt,
		YELLOW, RESET,
		BLACK,  RESET,
		MAGENTA, wd, RESET,
		BLUE, RESET
	);
}
