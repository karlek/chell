#define _prompt_h

/* Prompt format string. */
const char * prompt_fmt = "%s(^._.^)ﾉ%s %s@%s %s%s%s %s$%s ";

void print_prompt(char *wd, size_t size) {
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
