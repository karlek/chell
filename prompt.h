/* Prompt contains functions for printing the prompt. Also contains polling of
 stopped jobs.*/
#define _prompt_h

/* Prompt format string. */
const char * prompt_fmt = "%s(^._.^)ﾉ%s %s@%s %s%s%s %s$%s ";

/* poll checks for ended jobs.*/
void poll() {
	pid_t pid;

	/* Let parent process continue. */
	while((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
		fprintf(stdout, "[%d] job has ended.\n", pid);
	}
}

/* print_prompt prints the prompt in this manner:*/
/* (^._.^)ﾉ @ /CURRENT/WORKING/DIRECTORY $ */
void print_prompt(char *wd, size_t size) {
	/* If we want to check for ended processes with polling.*/
	if (!SIGDET) {
		poll();
	}

	/* Fix prompt. */
	if (getcwd(wd, size) == NULL) {
		fprintf(stderr, "getcwd: %s\n", strerror(errno));
	}

	/* Print prompt. */
	printf(
		prompt_fmt,
		YELLOW, RESET,
		BLACK,  RESET,
		MAGENTA, wd, RESET,
		BLUE, RESET
	);
}
