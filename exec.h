#define _exec_h

/* Execute command with arguments. Executes the command in a child process and
 waits for completion in the parent process. */
void execute(char **argv) {
	/* Child process pid. */
	pid_t pid;
	/* Status of the child. */
	int status;

	/* Fork a child process. */
	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork: forking child process failed.\n");
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
		waitpid(pid, &status, 0);
	}
}

/* background allows execution of commands in a child process.*/
void background(int argc, char **argv) {
	/* Signal handling. */
	struct sigaction sa;

	/* Executed command string. */
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
	/* Remove "&". */
	argv[argc-1] = NULL;

	if (SIGDET) {
		/* We want to recieve all signals by clearing the mask. */
		if (sigemptyset(&sa.sa_mask) == -1) {
			fprintf(stderr, "sigemptyset: failed - %s.\n", strerror(errno));
		}
		/* We want the program to continue execution. */
		sa.sa_flags = SA_RESTART;
		/* Callback function. */
		sa.sa_handler = sig_handler;
		/* Catch signal SIGCHLD and use callback function sig_handler.*/
		if (sigaction(SIGCHLD, &sa, NULL) == -1) {
			fprintf(stderr, "sigaction: failed - %s.\n", strerror(errno));
		}
	}

	/* For the child process. */
	if (fork() == 0)
	{
		/* We want to kill children. */
		if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
			fprintf(stderr, "signal: failed - %s\n", strerror(errno));
		}
		/* But we don't want to kill background processes with 'Ctrl-c'.*/
		if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
			fprintf(stderr, "signal: failed - %s\n", strerror(errno));
		}

		/* Execute the command. */
		execute(argv);

		if (SIGDET) {
			fprintf(stdout,"“%s” has ended.\n", command);
		}
		exit(0);
	}
}
