#define _signals_h

void sig_handler(int signo) {
	int ret;
	if (signo == SIGINT) {
		return;
	} else if(signo == SIGCHLD){
		printf("received SIGCHLD\n");
		ret = kill(getppid(), 2);
		if(ret < 0){
			printf("failed to send asd kill to %d", getppid());
		}else{
			printf("sending 1 to %d", getppid());
		}
		return;
	}
}

void handle_signals() {
	struct sigaction sa;

	sa.sa_handler = sig_handler;
	sigemptyset(&sa.sa_mask);
	/* Restart functions if interrupted by handler */
	sa.sa_flags = SA_RESTART;

	/* Handle error */
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		printf("error: sigaction.\n");
	}
}

/* ### unused atm. */
void handler(int signum) {
	/* Handler for SIGCHLD in background/2 */
	int status;
	wait(&status);
}
