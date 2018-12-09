#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char displayfd[7];
char *xserv_cmd[] = {"X", ":1", "-displayfd", displayfd, NULL};
char *xinit_cmd[] = {"sh", "/home/user/.xinitrc", NULL};

static pid_t xserv_pid = 0;
static pid_t xinit_pid = 0;
int signalpipe[2];

void handler(int s) {
	write(signalpipe[1], "", 1);
}

static void handle_signals(void (*func)(int)) {
	struct sigaction sa = {0};
	sa.sa_handler = handler;
	sa.sa_flags = func == handler ? SA_RESTART : 0;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
	sigaction(SIGCHLD, &sa, NULL);
}

static void cleanup() {
	if (xserv_pid > 0) kill(xserv_pid, SIGTERM);
	if (xinit_pid > 0) kill(xinit_pid, SIGTERM);
	if (xserv_pid > 0) waitpid(xserv_pid, NULL, 0);
	if (xinit_pid > 0) waitpid(xinit_pid, NULL, 0);
}

static void die(const char *msg) {
	fputs(msg, stderr);
	if (msg[0] && msg[strlen(msg)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
	
	cleanup();
	exit(EXIT_FAILURE);
}

static void start_xserv() {

	int fd[2];
	if (-1 == pipe(fd)) {
		die("pipe:");
	}

	snprintf(displayfd, sizeof(displayfd), "%d", fd[1]);

	xserv_pid = fork();
	if (xserv_pid == -1) {
		die("fork:");
	}
	
	if (xserv_pid == 0) {
		close(signalpipe[0]);
		close(signalpipe[1]);
		handle_signals(SIG_DFL);
		close(fd[0]);
		execvp(xserv_cmd[0], xserv_cmd);
		exit(1);
	}
	
	close(fd[1]);

	char display[10] = { ':', 0 };
	int n = read(fd[0], display + 1, sizeof(display) - 1);
	if (n == -1) {
		die("read:");
	}
	
	close(fd[0]);

	int ok = 0;
	int i;
	for (i = 0; i < n + 1; i++) {
		if (display[i] == '\n') {
			ok = 1;
			display[i] = '\0';
			break;
		}
	}

	if (!ok) {
		die("failed to read display number");
	}

	if (-1 == setenv("DISPLAY", display, 1)) {
		die("setenv:");
	}
}

static void start_xinit() {

	pid_t xinit_pid = fork();
	if (xinit_pid == -1) {
		die("fork:");
	}

	if (xinit_pid == 0) {
		close(signalpipe[0]);
		close(signalpipe[1]);
		handle_signals(SIG_DFL);
		execvp(xinit_cmd[0], xinit_cmd);
		exit(1);
	}
}


int main(int argc, char *argv[]) {

	if (-1 == pipe(signalpipe)) {
		die("pipe:");
	}

	handle_signals(handler);

	char *home = getenv("HOME");
	
	if (home == NULL) {
		die("HOME enviroment variable is not set");
	}
	
	if (-1 == chdir(home)) {
		die("chdir:");
	}
	
	start_xserv();
	start_xinit();

	char running = 1;
	while (running)	{
		read(signalpipe[0], &running, 1);
	}
	
	cleanup();
	
	return 0;
}
