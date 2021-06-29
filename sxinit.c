#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>


// Hardcode full path and arguments to Xorg to prevent users 
// of placing another binary into the search path,
// which would be executed with admin rights.
// For the same reason, it shouldn't be possible to submit 
// arbitrary arguments to xorg for users,
// nor should be Xorg itself be suid or executable by users.
// xorg is a quite complex executable, so I believe it's better to have 
// a small static binary like sxinit being suid,
// which can be checked for security flaws,
// with sametime known vulnerabilities of the xserver
//
#define XSERV_CMD { "/usr/bin/Xorg", "-nolisten", "tcp", "-noreset", "-displayfd", displayfd, 0 }


// the shell which parses $HOME/.xinitrc
// with the rights of the user calling sx
// The full path should be hardcoded for the same reasons
#define SHELL "/bin/sh"


// The default xinitrc, in case either HOME is not set,
// or HOME/.xinitrc not present
#define DEFAULT_XINITRC "/etc/X11/xinitrc"

#define PROGNAME "sxinit"
#define DISPLAY_ENV "DISPLAY=:?"
#define LENGTH(A) (sizeof(A) / sizeof((A)[0]))
#define SIGNALS {SIGTERM, SIGINT, SIGCHLD}

// globals
static pid_t xserv_pid;
static pid_t xinit_pid;
static sigset_t oldset;

extern char** environ;

void handler(int s) {
	return;
}

static void cleanup() {
	if (xserv_pid > 0) kill(xserv_pid, SIGTERM);
	if (xinit_pid > 0) kill(xinit_pid, SIGTERM);
	if (xserv_pid > 0) waitpid(xserv_pid, NULL, 0);
	if (xinit_pid > 0) waitpid(xinit_pid, NULL, 0);
}

// exit with failure
static void die(const char *msg) {
	int e = errno;
	fputs( PROGNAME ": ", stderr );
	fputs(msg, stderr);
	fputc( '\n', stderr );
	if ( e ){
		fputs( strerror(e),stderr);
		fputc( '\n', stderr );
	}

	cleanup();

	exit(EXIT_FAILURE);
}

// start the xserver
static void start_xserv() {
	int fd[2];
	if (pipe(fd))
		die("pipe:");

	char displayfd[2] = "?";
	displayfd[0] = '0'+fd[1];

	switch (xserv_pid = fork()) {
	case -1:
		die("fork:");
	case 0:
		close(fd[0]);
		sigprocmask(SIG_SETMASK, &oldset, NULL);
		char *xservcmd[] = XSERV_CMD;
		fputs( displayfd, stderr );
		fputc('\n',stderr);
		execve( xservcmd[0], xservcmd, environ );
		die("Couldn't start the xserver");
	}
	
	close(fd[1]);

	char display[16] = DISPLAY_ENV;

	// read the diplay fd number and set the enviromental variable
	int n = read(fd[0], display+sizeof(DISPLAY_ENV)-2, 7);

	fputs(display,stderr);

	if (n == -1){
		die("Couldn't read from pipe");
	}
	
	close(fd[0]);

	for (int k = sizeof(DISPLAY_ENV)-1; k < n + sizeof(DISPLAY_ENV); k++) {
		if ( (display[k] == '\n') || ( display[k] == 0 ) ) {
			display[k] = '\0';
			if (putenv(display))
				die("putenv:");
			return;
		}
	}

	die("failed to read display number");
}

// execute xinitrc
static void start_xinit(int argc, char *argv[]) {
	xinit_pid = fork();
	if (xinit_pid == -1)
		die("fork:");

	if (xinit_pid) // parent
		return;

	sigprocmask(SIG_SETMASK, &oldset, NULL);

	char buf[PATH_MAX];

	char *xinitcmd[argc+3];
	xinitcmd[0] = SHELL;
	xinitcmd[1] = buf;

	// copy arguments
	char **xp = xinitcmd+2;
	while( ( *xp = *argv ) ){
		xp++; argv++;
	}

	char *home = getenv("HOME");
	if ( home ){
		char *c = stpcpy(buf,home);
		strcpy(c,"/.xinitrc");
		execve( xinitcmd[0], xinitcmd, environ);
	} else {
		fputs("HOME variable is not set.\n",stderr);
	}

	fputs("Using " DEFAULT_XINITRC "\n",stderr);

	// either HOME is not set, or no xinitrc present
	strcpy(buf,DEFAULT_XINITRC);

	execve( xinitcmd[0], xinitcmd, environ);

	// shouldn't get here
	die("Couldn't execute xinitrc");
}



int main(int argc, char *argv[]) {
	struct sigaction sa = {0};
	sa.sa_handler = handler;
	int signals[] = SIGNALS;

	if (sigemptyset(&sa.sa_mask))
		die("sigemptyset:");
	for (int i = 0; i < LENGTH(signals); i++)
		if (sigaddset(&sa.sa_mask, signals[i]))
			die("sigaddset:");
	if (sigprocmask(SIG_BLOCK, &sa.sa_mask, &oldset))
		die("sigprocmask:");

	char *home = getenv("HOME");
	if (home == NULL)
		die("HOME enviroment variable is not set");
	
	if (chdir(home))
		die("chdir:");

	start_xserv();
	fputs( PROGNAME ": Xorg started\n",stderr);

	// Drop suid privileges
	setreuid( -1, getuid() );

	argv++;
	start_xinit(argc-1,argv);


	for (int i = 0; i < LENGTH(signals); i++)
		if (sigaction(signals[i], &sa, NULL))
			die("sigaction:");

	sigsuspend(&oldset);
	if (errno != EINTR)
		die("sigsuspend:");

	cleanup();

	return EXIT_SUCCESS;
}
