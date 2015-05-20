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
	struct sigaction sa;
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

<<<<<<< HEAD
	if((pid = fork()) == 0)
	{
		sigset(SIGCHLD, sig_handler);
		sighold(SIGCHLD);
		execute(argv);
		fprintf(stdout,"\n“%s” has ended.\n", command);
		sigrelse(SIGCHLD);
		exit(0);
	}
	kill(getpid(), SIGCHLD);
=======
	/* ### WTF? */
	if(SIGDET) {
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = sig_handler;
		if(sigaction(SIGCHLD, &sa, NULL)== -1) {
			fprintf(stderr, "sigaction failed\n");
		}
	}

	/* For the child process. */
	if((pid = fork()) == 0)
	{
		/* We want to kill children. */
		signal(SIGQUIT, SIG_DFL);

		/* Execute the command. */
		printf("\n");
		execute(argv);

		if (SIGDET) {
			fprintf(stdout,"“%s” has ended.\n", command);
		}
		/* ### might be bad that we return zero. Should return executes ret? */
		exit(0);
	}
>>>>>>> 19be449d03a505bf28c9ec5cae2c48d90e9b99b5
}
