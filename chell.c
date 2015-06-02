/* Chell is a homage to the protagonist in the portal games. */

#ifndef SIGDET
	#define SIGDET 0
#endif

#include "command.h"
#include "prompt.h"
#include "parse.h"
#include "signals.h"
#include "exec.h"

#define INP_LEN 256

void interpret(int, char **, char *, size_t);
void get_line(char *, size_t);

void get_input(char *input, size_t size) {
	int i;
	char c;
	char *start = input;
	for (i = 0; i < (int)size; i++) {
		/* Read input. */
		c = getc(stdin);
		/* -1 == EOF */
		if (c == -1) {
			/* Quit chell if no command is inputted. */
			if(strlen(start) == 0) {
				kill(0, SIGQUIT);
				exit(0);
			}
			/* Otherwise ignore this. */
			continue;
		}
		/* Add input to buffer. */
		*input++ = c;
		if (c == '\n') {
			break;
		}
	}
}

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
