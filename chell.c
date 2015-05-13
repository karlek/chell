/* Chell is a homeage to the protagonist in the portal games. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define INP_LEN 256
#define CMD_LEN 256

const char * prompt = "(^._.^)ﾉ > ";

void cd(char *);
int index(char *, char);
char * words(char *);
int system(char *);
void exit(int);
char * getenv(char *);

int main(int argc, char const *argv[])
{
	/* Whole input string. */
	char input[INP_LEN];
/*	char tokenizer[INP_LEN];*/
	
	/* Command to execute. */
	char command[CMD_LEN];

	/* Working directory. */
	char wd[256];

	/*char * pch;*/
	char * pager;
	char argument[256];
	int ind = 0, ret = 0;

	while (1) {
		/* Print prompt. */
		printf("%s", prompt);
	
		/* Get user input. */
		fgets(input, CMD_LEN, stdin);

		ind = index(input, ' ');
		if (ind != -1) {
			strncpy(command, input, ind);
			command[strlen(command)] = '\0';
		} else {
			strncpy(command, input, sizeof(command));
			command[strlen(command)-1] = '\0';
		}

		/* a built-in command "exit" which terminates all remaining processes
		started from the shell in an orderly manner before exiting the shell
		itself */

		printf("command: `%s`\n", command);
		if (strcmp("exit", command) == 0) {
			exit(0);
		} else if (strcmp("cd", command) == 0) {
			printf("%s\n", input);
			/* Ignore the space and the command. */
			if (ind != -1) {
				cd(input+ind+1);			
			} else {
				cd(input+strlen(command)+1);			
			}
		} else if (strcmp("pwd", command) == 0) {
			if (getcwd(wd, sizeof(wd)) != NULL) {
			    fprintf(stdout, "Current working dir: %s\n", wd);
			} else {
			    printf("getcwd() error");
			}
	    } else if (strcmp("checkEnv", command) == 0) {
	    	pager = getenv("PAGER");
	    	if (pager == NULL) {
	    		pager = "lessq";	
	    	}
	    	printf("pager: %s\n", pager);
	    	sprintf(argument, "printenv | sort | %s", pager);
	    	printf("arg: %s\n", argument);
	    	ret = system(argument);
	    	printf("ret: %d\n", ret);
	    	if (ret != 0) {
	    		pager = "more";
	    	}
	    	printf("here\n");
	    	sprintf(argument, "printenv | sort | %s", pager);
	    	system(argument);
	    } else {
	    	/* printf("input: %s\n", input); */
	    	system(input);
	    }
	
		/* Argument number. */
		/* i = 0; */

		/* Still exists an argument. */
		/*while(pch != NULL) {*/
			/* The first argument is always a command. */
/*			if (i == 0) {}
			pch = strtok (NULL, " ,.-");
			i++;
		}
*/	
		/* Null terminated strings for strcmp. 
		ind = index(input, '\n');
		if (ind == -1) {
			ind = INP_LEN-1;
		}
		input[ind] = '\0';
		*/

		/* Zero the strings. */
		memset(command, 0, strlen(command));
		memset(input, 0, strlen(input));
	}
	return 0;
}

int index(char * buf, char c) {
	int index = -1;
	char * ptr;
	ptr = strchr(buf, c);
	if(ptr) {
		index = ptr - buf;
	}
	return index;
}

char * words(char * str) {
	char * pch;
	pch = strtok(str, " \n");
	return pch;
}

void cd(char * input) {
	int ret = 0;
	printf("cd: `%s`\n", input);
	printf("strlen: %u\n", (int)strlen(input));
	if ((int)strlen(input) == 0) {
		chdir(getenv("HOME"));
		return;
	}
	/* Remove newline. */
	input[strlen(input)-1] = '\0';

	ret = chdir(input);
	switch (ret) {
	case -1:	
		printf("cd: The directory “%s” does not exist\n", input);
	}
}