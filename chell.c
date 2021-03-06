/* Chell is a homage to the protagonist in the portal games. */

#ifndef SIGDET
#define SIGDET 0
#endif

#define NAME "chell"

/* Needed for sigrelse, sighold, snprintf and kill. */
#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "colors.h"
#include "prompt.h"
#include "parse.h"
#include "signals.h"
#include "exec.h"
#include "commands/pwd.h"
#include "commands/cd.h"
#include "commands/checkEnv.h"

#define INP_LEN 256

void interpret(int, char **, char *, size_t);

int main(int argc, char const *argv[]) {
	/* Whole input line. */
	char input[INP_LEN] = "";

	/* String token array. For easier retrieval of commands arguments.*/
	char *args[32];

	/* Working directory. */
	char wd[1024];

	/* Number of string tokens read. */
	int nwords = 0;

	/* Prevent inner-shell to kill outer-shell. exit kills all in process group.*/
	signal(SIGQUIT, SIG_IGN);
	handle_signals();

	while (1) {
		print_prompt(wd, sizeof(wd));

		/* Get user input. */
		get_input(input, sizeof(input));
		/* Get string tokens. */
		nwords = parse(input, args, 32);
		switch (nwords) {
		case -1:
			fprintf(stderr, "%s: too many arguments.\n", NAME);
		case 0:
			/* Defer would be nice, eh?*/
			memset(input, 0, strlen(input));
			continue;
		}

		interpret(nwords, args, wd, sizeof(wd));

		/* Zero the strings. */
		memset(input, 0, strlen(input));
	}
	return 0;
}

void interpret(int argc, char **argv, char *wd, size_t size) {
	/* Execution time variables. */
	struct timeval begin, end;

	/* Total execution time (end-begin). */
	long elapsed;

	/* Interpret the entered command. */
	if (strcmp("exit", argv[0]) == 0) {
		/* exit kills the shell. */
		kill(0, SIGQUIT);
		exit(0);
	} else if (strcmp("cd", argv[0]) == 0) {
		/* cd changes the directory. */
		cd(argv[1]);
	} else if (strcmp("pwd", argv[0]) == 0) {
		/* pwd prints the working directory. */
		pwd(wd, size);
	} else if (strcmp("checkEnv", argv[0]) == 0) {
		/* checkEnv prints a sorted list of all environmental variables.*/
		checkEnv(argc, argv);
	} else if (strcmp("&", argv[argc-1]) == 0) {
		/* background runs the command in a background process. */
		background(argc, argv);
	} else {
		/* run the command in the foreground and measure it's execution time.*/
		gettimeofday(&begin, 0);
		execute(argv);
		gettimeofday(&end, 0);
		elapsed = (end.tv_sec-begin.tv_sec)*1000000 + end.tv_usec-begin.tv_usec;
		printf("time: %.3fs\n", (double)elapsed/1000000);
	}
}
