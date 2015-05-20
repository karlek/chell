#define _command_h

/* Needed for sigrelse, sighold, snprintf and kill. */
#define _XOPEN_SOURCE 500

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "colors.h"

#define NAME "chell"

#define STDERR 2
#define STDOUT 1
#define STDIN 0

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

char *get_pager() {
	/* Pid of the `which less` test. */
	pid_t pid_less;
	/* Status of the child process. */
	int status;

	char *pager = getenv("PAGER");

	if (pager == NULL) {
		pager = "less";
	}
	if ((pid_less = fork()) == 0){
		exists(pager);
	}
	while (wait(&status) != pid_less);
	if (status == 256) {
		pager = "more";
	}
	return pager;
}

void close_all(int *pipes, int n) {
	int i;
	for (i = 0; i < n; ++i) {
		close(pipes[i]);
	}
}

void checkEnv(int argc, char **grep_args) {
	/* Commands to run. */
	char *printenv_args[] = {"printenv", NULL};
	char *sort_args[] = {"sort", NULL};
	char *pager_args[] = {"pager", NULL};

	/* Piping stuff. */
	int status, i;

	int pipes[6];

	/* we now have 4 fds: */
	/* pipes[0] = read end of cat->grep pipe (read by grep) */
	/* pipes[1] = write end of cat->grep pipe (written by cat) */
	/* pipes[2] = read end of grep->cut pipe (read by cut) */
	/* pipes[3] = write end of grep->cut pipe (written by grep) */

	/* Note that the code in each if is basically identical, so you */
	/* could set up a loop to handle it.  The differences are in the */
	/* indicies into pipes used for the dup2 system call */
	/* and that the 1st and last only deal with the end of one pipe. */


	/* Decide pager program. */
	pager_args[0] = get_pager();
	grep_args[0] = "grep";

	/* Create a pipe. */
	if(-1 == pipe(pipes)) {
		fprintf(stderr,"PIPE error: %s\n", strerror(errno));
	}
	if(-1 == pipe(pipes+2)) {
		fprintf(stderr,"PIPE+2 error: %s\n", strerror(errno));
	}
	if(-1 == pipe(pipes+4)) {
		fprintf(stderr,"PIPE+4 error: %s\n", strerror(errno));
	}

	printf("args: %s\n", grep_args[1]);
	printf("pager: %s\n", pager_args[0]);

	if(fork() == 0) {
		dup2(pipes[1], STDOUT);

		close_all(pipes, 6);

		execvp(printenv_args[0], printenv_args);
	}

	if(fork() == 0) {
		/* Read from stdin. */
		dup2(pipes[0], STDIN);

		/* Write to stdout. */
		dup2(pipes[3], STDOUT);

		close_all(pipes, 6);

		execvp(sort_args[0], sort_args);
	}

	if (grep_args[1] != NULL){
		if(fork() == 0) {
			/* Stdin. */
			dup2(pipes[2], STDIN);

			/* Stdout. */
			dup2(pipes[5], STDOUT);

			close_all(pipes, 6);

			execvp(grep_args[0], grep_args);
		}
		if(fork() == 0) {
			/* Stdin. */
			dup2(pipes[4], STDIN);

			close_all(pipes, 6);

			execvp(pager_args[0], pager_args);
		}
	} else {
		if(fork() == 0) {
			/* Stdin. */
			dup2(pipes[2], STDIN);

			close_all(pipes, 6);

			execvp(pager_args[0], pager_args);
		}
	}

	close_all(pipes, 6);

	/* ### If we don't grep anything, should this number be 3? */
	for (i = 0; i < 3; i++) {
		wait(&status);
	}
	if(grep_args[1] == NULL){
		wait(&status);
	}
}

/* Doesn't work for ~root */
void cd(char * input) {
	int ret = 0;
	char path[256] = "";

	/* No argument -> go to home directory. */
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

void pwd(char *wd, size_t size) {
	if (getcwd(wd, size) != NULL) {
		printf("Current working dir: %s\n", wd);
	} else {
		fprintf(stderr, "getcwd: %s\n", strerror(errno));
	}
}
