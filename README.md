# chell

## Installation

```shell
$ git clone git@github.com:karlek/chell.git
$ cd chell
$ make
# or make sigdet (if you prefer signal determination over polling)
```

## Usage

```shell
$ alias chell="rlwrap -q '"\'' -H '~/.chell-hist' -D -i -c -A ./chell"
$ chell
```

## Constraints

* The lab should be written in ANSI standard C. (the reason for this is that one can always assume that even old systems adhere to this standard and the differences to more recent standards as C99 is not that huge)

* The return value from all system calls have to be checked for errors.

* All termination of processes started from the shell must be handled correctly as not to leave a possibility for creating zombie-processes

* The code should compile without warnings on shell.ict.kth.se/shell.it.kth.se when compiled with the command "gcc -pedantic -Wall -ansi -O4"

* The shell should behave controlled, i.e. as your normal shell (bash, tcsh) for all reasonable input to the shell and to processes started from the shell (for the latter check what happens if you terminate a child process with Ctrl-C"

* You are not allowed to keep lists of your own for background processes or communicate with a signal handler by global variables. This simple shell does not require that - this restriction forces you to better understand what the OS is doing (i.e.do not do anything unneccessary)

* You may use the POSIX library functions sighold() and sigrelse() if available.

* You need not be able to handle longer command-line input than 80 characters.

* You are not allowed to use excessive memory or have "memory leaks" (i.e. the amount of memory allocated through malloc(3C) may not grow over time - however you probably do not need to allocate any memory in this way)
