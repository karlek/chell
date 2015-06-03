#define _checkEnv_h

#define STDERR 2
#define STDOUT 1
#define STDIN 0

/* exists is used with fork and wait(&status) to determine if a command exists.*/
void exists(char *command) {
	/* /dev/null file-descriptor. */
	int fd;

	/* Command to execute. */
	char *command_args[] = {"placeholder", NULL};

	/* Which command exists? */
	command_args[0] = command;

	/* Pipe to /dev/null. */
	fd = open("/dev/null", O_WRONLY);

	/* Redirect stdout. */
	dup2(fd, STDOUT);
	/* Redirect stderr. */
	dup2(fd, STDERR);
	/* fd no longer needed - dup knows whats up. */
	close(fd);

	/* Run the command. */
	if (-1 == execvp(command_args[0], command_args)) {
		exit(1);
	}
}

/* get_pager returns a pager command string. Firstly it uses the environmental
 variable `$PAGER`, if it's not set it's set to `less` and finally if `less` is
 not available it defaults to `more`.*/
char *get_pager() {
	/* Pid of the `which less` test. */
	pid_t pid_less;
	/* Status of the child process. */
	int status;

	char *pager = getenv("PAGER");
	if (pager == NULL) {
		pager = "less";
	}
	if ((pid_less = fork()) == 0) {
		exists(pager);
	}
	/* Wait for fork return. */
	while (wait(&status) != pid_less);
	if (status == 256) {
		pager = "more";
	}
	return pager;
}

/* Takes a int list of pipes and closes them.*/
void close_all(int *pipes, int n) {
	int i;
	for (i = 0; i < n; ++i) {
		close(pipes[i]);
	}
}

/* Create pipes. */
void create_pipes(int pipes[6]) {
	if (-1 == pipe(pipes)) {
		fprintf(stderr,"PIPE error: %s\n", strerror(errno));
	}
	if (-1 == pipe(pipes+2)) {
		fprintf(stderr,"PIPE+2 error: %s\n", strerror(errno));
	}
	if (-1 == pipe(pipes+4)) {
		fprintf(stderr,"PIPE+4 error: %s\n", strerror(errno));
	}
}

/* checkEnv pretty prints all environmental variables and sorts them. If a
 optional argument is passed, the environmental variables are filtered with
 `grep argument`.*/
void checkEnv(int argc, char **grep_args) {
	/* Commands to run. */
	char *printenv_args[] = {"printenv", NULL};
	char *sort_args[] = {"sort", NULL};
	/* pager here is placeholder.*/
	char *pager_args[] = {"pager", NULL};

	/* Piping stuff. */
	int status, i, num_proc = 3;
	/* we now have 6 fds: */
	/* the pipes distribution is dependent on any optional grep arguments. */
	/* */
	/* pipes[0] = read end of printenv->sort pipe (read by sort) */
	/* pipes[1] = write end of printenv->sort pipe (written by printenv) */
	/* pipes[2] = read end of sort->grep/pager pipe (read by grep/pager) */
	/* pipes[3] = write end of sort->grep/pager pipe (written by sort) */
	/* */
	/* This only happens if we have a filter argument to grep.*/
	/* pipes[4] = write end of grep->pager pipe (written by grep) */
	/* pipes[5] = write end of grep->pager pipe (written by grep) */
	int pipes[6];

	if (argc > 1) {
		num_proc = 4;
	}

	/* Decide pager program. */
	pager_args[0] = get_pager();
	grep_args[0] = "grep";

	create_pipes(pipes);

	if (fork() == 0) {
		/* Write stdout of printenv to sort (pipes[1]). */
		dup2(pipes[1], STDOUT);
		close_all(pipes, 6);
		execvp(printenv_args[0], printenv_args);
	}

	if (fork() == 0) {
		/* Read stdin to sort from stdout of printenv (pipes[0]). */
		dup2(pipes[0], STDIN);

		/* Write stdout of sort to either grep or pager. */
		dup2(pipes[3], STDOUT);
		close_all(pipes, 6);
		execvp(sort_args[0], sort_args);
	}

	/* If we have grep arguments, filter than show with pager.*/
	if (grep_args[1] != NULL) {
		if (fork() == 0) {
			/* Read from sort stdout.*/
			dup2(pipes[2], STDIN);
			/* Write stdout of grep to pager.*/
			dup2(pipes[5], STDOUT);
			close_all(pipes, 6);
			execvp(grep_args[0], grep_args);
		}
		if (fork() == 0) {
			/* Read from grep stdin. */
			dup2(pipes[4], STDIN);
			close_all(pipes, 6);
			execvp(pager_args[0], pager_args);
		}
	} else {
		/* If we don't have any grep arguments, we don't need to filter.*/
		if (fork() == 0) {
			/* Read from sort stdin. */
			dup2(pipes[2], STDIN);
			close_all(pipes, 6);
			execvp(pager_args[0], pager_args);
		}
	}

	close_all(pipes, 6);

	/* We always have three forks.*/
	/* And sometimes we have a grep fork.*/
	for (i = 0; i < num_proc; i++) {
		if (wait(&status) == -1) {
			fprintf(stderr, "wait: failed - %s\n", strerror(errno));
		}
	}
}

