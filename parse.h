#define _parse_h

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
