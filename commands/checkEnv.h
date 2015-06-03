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
	char **args[4];
	char *printenv_args[] = {"printenv", NULL};
	char *sort_args[] = {"sort", NULL};
	char *pager_args[] = {"pager", NULL};

	/* Piping stuff. */
	int i = 0, num_proc = 3;

	/* we now have 6 fds:
	   the pipes distribution is dependent on any optional grep arguments.

	   pipes[0] = read end of printenv->sort pipe (read by sort)
	   pipes[1] = write end of printenv->sort pipe (written by printenv)
	   pipes[2] = read end of sort->grep/pager pipe (read by grep/pager)
	   pipes[3] = write end of sort->grep/pager pipe (written by sort)

	   This only happens if we have a filter argument to grep
	   pipes[4] = write end of grep->pager pipe (written by grep)
	   pipes[5] = write end of grep->pager pipe (written by grep)
	*/
	int pipes[6];

	/* Child pids.*/
	pid_t pid;

	/* If we have any arguments, use grep.*/
	if (argc > 1) {
		num_proc = 4;
	}

	/* Decide pager program. */
	pager_args[0] = get_pager();
	grep_args[0] = "grep";

	/* Fill args.*/
	args[i++] = printenv_args;
	if (num_proc == 4) {
		args[i++] = grep_args;
	}
	args[i++] = sort_args;
	args[i++] = pager_args;

	create_pipes(pipes);

	for (i = 0; i < num_proc; i++) {
		if ((pid = fork()) < 0) {
			fprintf(stderr, "fork: forking child process failed.\n");
			exit(1);
		} else if (pid != 0) {
			break;
		}
		/* We don't read from stdin first pipe.*/
		if (i != 0) {
			dup2(pipes[(i-1)*2], STDIN);
		}
		/* We don't pipe stdout last pipe.*/
		if (i != num_proc-1) {
			dup2(pipes[i*2+1], STDOUT);
		}
		close_all(pipes, 6);

		execvp(*args[i], args[i]);
	}
	close_all(pipes, 6);

	/* We always have three forks.*/
	/* And sometimes we have a grep fork.*/
	for (i = 0; i < num_proc; i++) {
		if (wait(NULL) == -1) {
			fprintf(stderr, "wait: failed - %s\n", strerror(errno));
		}
	}
}

