#define _parse_h

/* get_input reads at maximum size many chars into input. */
void get_input(char *input, size_t size) {
	/* Buffer overflow limit. */
	int i;
	/* Current char. */
	char c;
	/* Used for strlen(start). */
	char *start = input;

	/* Read size many chars or until we recive '\n' / ^D. */
	for (i = 0; i < (int)size; i++) {
		/* Read input. */
		c = getc(stdin);
		/* -1 == EOF */
		if (c == -1) {
			/* Quit chell if no command is inputted. */
			if (strlen(start) == 0) {
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

/* parse splits line into words / args. At maximum 32 tokens / arguments.*/
int parse(char *line, char *argv[32], size_t size) {
	/* Number of args. */
	int argc = 0;
	char **tmp = argv;

	/* Skip leading whitespaces */
	while (isspace(*line)) {
		line++;
	}

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
		while (isspace(*line)) {
			line++;
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

	/* Add NULL as last element. */
	tmp[argc] = NULL;
	return argc;
}
