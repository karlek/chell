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
	if ((pid_less = fork()) == 0){
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
	int status, i;
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

	/* Decide pager program. */
	pager_args[0] = get_pager();
	grep_args[0] = "grep";

	/* Create pipes. */
	if(-1 == pipe(pipes)) {
		fprintf(stderr,"PIPE error: %s\n", strerror(errno));
	}
	if(-1 == pipe(pipes+2)) {
		fprintf(stderr,"PIPE+2 error: %s\n", strerror(errno));
	}
	if(-1 == pipe(pipes+4)) {
		fprintf(stderr,"PIPE+4 error: %s\n", strerror(errno));
	}

	if(fork() == 0) {
		/* Write stdout of printenv to sort (pipes[1]). */
		dup2(pipes[1], STDOUT);
		close_all(pipes, 6);
		execvp(printenv_args[0], printenv_args);
	}

	if(fork() == 0) {
		/* Read stdin to sort from stdout of printenv (pipes[0]). */
		dup2(pipes[0], STDIN);

		/* Write stdout of sort to either grep or pager. */
		dup2(pipes[3], STDOUT);
		close_all(pipes, 6);
		execvp(sort_args[0], sort_args);
	}

	/* If we have grep arguments, filter than show with pager.*/
	if (grep_args[1] != NULL){
		if(fork() == 0) {
			/* Read from sort stdout.*/
			dup2(pipes[2], STDIN);
			/* Write stdout of grep to pager.*/
			dup2(pipes[5], STDOUT);
			close_all(pipes, 6);
			execvp(grep_args[0], grep_args);
		}
		if(fork() == 0) {
			/* Read from grep stdin. */
			dup2(pipes[4], STDIN);
			close_all(pipes, 6);
			execvp(pager_args[0], pager_args);
		}
	} else {
		/* If we don't have any grep arguments, we don't need to filter.*/
		if(fork() == 0) {
			/* Read from sort stdin. */
			dup2(pipes[2], STDIN);
			close_all(pipes, 6);
			execvp(pager_args[0], pager_args);
		}
	}

	close_all(pipes, 6);

	/* We always have three forks.*/
	for (i = 0; i < 3; i++) {
		if (wait(&status) == -1) {
			fprintf(stderr, "wait: failed - %s\n", strerror(errno));
		}
	}
	/* And sometimes we have a grep fork.*/
	if(grep_args[1] == NULL){
		if (wait(&status) == -1) {
			fprintf(stderr, "wait: failed - %s\n", strerror(errno));
		}
	}
}

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

/* pwd prints the current working directory. */
void pwd(char *wd, size_t size) {
	if (getcwd(wd, size) != NULL) {
		printf("Current working dir: %s\n", wd);
	} else {
		fprintf(stderr, "getcwd: %s\n", strerror(errno));
	}
}
