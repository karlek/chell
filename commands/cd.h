#define _cd_h

/* cd changes the current working directory to user input.*/
void cd(char * input) {
	int ret = 0;
	char path[256] = "";

	/* No argument -> go to home directory. */
	/* Strings prefixed with '~' are user home folders. */
	/* Otherwise try to goto that folder. */
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

