/* signals.h implements a signal handler */
#define _signals_h

/* sig_handler handles SIGCHLD and SIGINT signals.
*/
void sig_handler(int signo) {
	if (signo == SIGINT) {
		return;
	} else if(signo == SIGCHLD){
		waitpid(-1, NULL, WNOHANG);
	}
}

/* Only handles SIGINT. */
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