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

		/* We want to kill children. */
		signal(SIGQUIT, SIG_DFL);
		/* Execute the command. */
		if (execvp(*argv, argv) < 0) {
			fprintf(stderr, "%s: Unknown command: %s\n", NAME, argv[0]);
			exit(1);
		}
	} else {
		/* For the parent. */

		/* Wait for completion. */
		while (wait(&status) != pid);
	}
}

void background(int argc, char **argv) {
	pid_t pid;
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
	argv[argc-1] = NULL;

	if((pid = fork()) == 0)
	{
		if(SIGDET) {
			sigset(SIGCHLD, sig_handler);
		}
		sighold(SIGCHLD);
		execute(argv);
		if (SIGDET) {
			fprintf(stdout,"\n“%s” has ended.\n", command);
		}
		sigrelse(SIGCHLD);
		exit(0);
	}
	kill(getpid(), SIGCHLD);
}
