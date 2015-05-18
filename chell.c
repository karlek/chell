/* Chell is a homage to the protagonist in the portal games. */

/* Needed for sigrelse, sighold, snprintf and kill. */
#define _XOPEN_SOURCE 500

#include <ctype.h>
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
	/* Handler for SIGCHLD in background/2 */
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

int parse(char *line, char *argv[32], size_t size) {
	/* Number of args. */
	int argc = 0;
	char **tmp = argv;

	/* Skip leading whitespaces */
	while(isspace(*line)) line++;

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
		if (*line == ' ' ||
			   *line == '\n') {
			argc++;
			*line++ = '\0';
		}

		/* Skip internal whitespaces */
		while(isspace(*line)) line++;

		/* Save the argument. */
		*argv++ = line;

		/* Skip until we need to remove whitespace.  */
		while (*line != '\0' &&
			   *line != ' '  &&
			   *line != '\n') {
			line++;
		}
	}

	/* Add NULL as last element. */
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

void exists(char * command) {
	/* /dev/null file-descriptor. */
	int fd;

	/* Command to execute. */
	char *which_args[] = {"which", "placeholder", NULL};

	/* Which command exists? */
	which_args[1] = command;

	/* Pipe to /dev/null. */
	fd = open("/dev/null", O_WRONLY);

	/* Redirect stdout. */
	dup2(fd, 1);
	/* Redirect stderr. */
	dup2(fd, 2);
	/* fd no longer needed - dup knows whats up. */
	close(fd);

	/* Run the command. */
	if (-1 == execvp(*which_args, which_args)) {
		exit(1);
	}
}

char *get_pager() {
	/* Pid of the `which less` test. */
	pid_t pid_less;
	/* Status of the child process. */
	int status;

	char *pager = getenv("PAGER");

	if (pager == NULL) {
		pager = "less";
	}
	if ((pid_less = fork()) == 0){
		exists(pager);
	}
	while (wait(&status) != pid_less);
	if (status == 256) {
		pager = "more";
	}
	return pager;
}

/* Still broken, needs to be implemented with pipe. */
void checkEnv(int argc, char **grep_args) {
	/* Commands to run. */
	char *printenv_args[] = {"printenv", NULL};
	char *sort_args[] = {"sort", NULL};
	char *pager_args[] = {"pager", NULL};

	/* Piping stuff. */
	int status, i;

	int pipes[6];
	pipe(pipes); /* sets up 1st pipe */
	pipe(pipes + 2); /* sets up 2nd pipe */
	pipe(pipes + 4); /* sets up 2nd pipe */


	/* we now have 4 fds: */
	/* pipes[0] = read end of cat->grep pipe (read by grep) */
	/* pipes[1] = write end of cat->grep pipe (written by cat) */
	/* pipes[2] = read end of grep->cut pipe (read by cut) */
	/* pipes[3] = write end of grep->cut pipe (written by grep) */

	/* Note that the code in each if is basically identical, so you */
	/* could set up a loop to handle it.  The differences are in the */
	/* indicies into pipes used for the dup2 system call */
	/* and that the 1st and last only deal with the end of one pipe. */


	/* Decide pager program. */
	pager_args[0] = get_pager();
	grep_args[0] = "grep";

	/* Create a pipe. */
	if(-1 == pipe(pipes)) {
		printf("\n\n\n\n\nerror!\n\n\n\n\n");
	}
	if(-1 == pipe(pipes+2)) {
		printf("\n\n\n\n\nerror!\n\n\n\n\n");
	}

	printf("args: %s\n", grep_args[1]);
	printf("pager: %s\n", pager_args[0]);

	if(fork() == 0) {
		dup2(pipes[1], 1);

		close(pipes[0]);
		close(pipes[1]);
		close(pipes[2]);
		close(pipes[3]);
		close(pipes[4]);
		close(pipes[5]);

		execvp(printenv_args[0], printenv_args);
	}

	if(fork() == 0) {
		/* Read from stdin. */
		dup2(pipes[0], 0);

		/* Write to stdout. */
		dup2(pipes[3], 1);

		close(pipes[0]);
		close(pipes[1]);
		close(pipes[2]);
		close(pipes[3]);
		close(pipes[4]);
		close(pipes[5]);

		execvp(sort_args[0], sort_args);
	}

	if (grep_args[1] != NULL){
		if(fork() == 0) {
			/* Stdin. */
			dup2(pipes[2], 0);

			/* Stdout. */
			dup2(pipes[5], 1);

			close(pipes[0]);
			close(pipes[1]);
			close(pipes[2]);
			close(pipes[3]);
			close(pipes[4]);
			close(pipes[5]);

			execvp(grep_args[0], grep_args);
		}
		if(fork() == 0) {
			/* Stdin. */
			dup2(pipes[4], 0);

			close(pipes[0]);
			close(pipes[1]);
			close(pipes[2]);
			close(pipes[3]);
			close(pipes[4]);
			close(pipes[5]);

			execvp(pager_args[0], pager_args);
		}
	} else {
		if(fork() == 0) {
			/* Stdin. */
			dup2(pipes[2], 0);

			close(pipes[0]);
			close(pipes[1]);
			close(pipes[2]);
			close(pipes[3]);
			close(pipes[4]);
			close(pipes[5]);

			execvp(pager_args[0], pager_args);
		}
	}

	close(pipes[0]);
	close(pipes[1]);
	close(pipes[2]);
	close(pipes[3]);
	close(pipes[4]);
	close(pipes[5]);

	for (i = 0; i < 3; i++) {
		wait(&status);
	}
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
