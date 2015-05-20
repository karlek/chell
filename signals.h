/* signals.h implements a signal handler */
#define _signals_h

/* sig_handler handles SIGCHLD and SIGINT signals.
*/
void sig_handler(int signo) {
	if(signo == SIGCHLD){
		waitpid(-1, NULL, WNOHANG);
	} else if (signo == SIGINT) {
		return;
	}
}

/* Only handles SIGINT. */
void handle_signals() {
	struct sigaction sa;

	/* We want to recieve all signals by clearing the mask. */
	if (sigemptyset(&sa.sa_mask) == -1) {
		fprintf(stderr, "sigemptyset: failed - %s.\n", strerror(errno));
	}
	/* Restart functions if interrupted by handler */
	sa.sa_flags = SA_RESTART;
	/* sig_handler is our callback function for SIGINT. */
	sa.sa_handler = sig_handler;

	/* Listen for SIGINT signals. */
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		printf("sigaction: failed - %s.\n", strerror(errno));
	}
}