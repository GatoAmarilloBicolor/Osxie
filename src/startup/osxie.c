/*
This file is part of Osxie.

Copyright (C) 2016-2023 Lubos Dolezel

Osxie is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Osxie is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Osxie.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdbool.h>

static volatile sig_atomic_t timeout_reached = 0;

void alarm_handler(int sig)
{
	timeout_reached = 1;
}
#include <sched.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <getopt.h>
#include <termios.h>
#include <pty.h>
#include <pwd.h>
#include "../shellspawn/shellspawn.h"
#include "osxie.h"
#include "osxie-config.h"

char *prefix;
uid_t g_originalUid, g_originalGid;
bool g_fixPermissions = false;
char g_workingDirectory[4096];
pid_t pidInit;

// Kill the whole container. The launchd process is PID 1 of the container's
// PID namespace; killing it with SIGKILL makes the kernel reap every other
// process in that namespace, which is the only reliable way to tear down a
// container whose children have been reparented or detached.
void killOsxieContainer(void)
{
	if (pidInit <= 0)
		return;

	pid_t launchd_pid = -1;
	char path_buf[128];
	snprintf(path_buf, sizeof(path_buf), "/proc/%d/task/%d/children", pidInit, pidInit);
	FILE* file = fopen(path_buf, "r");
	if (file) {
		if (fscanf(file, "%d", &launchd_pid) != 1)
			launchd_pid = -1;
		fclose(file);
	}

	if (launchd_pid > 0)
		kill(launchd_pid, SIGKILL);
	kill(pidInit, SIGKILL);
}

int main(int argc, char ** argv)
{

	if (argc <= 1)
	{
		showHelp(argv[0]);
		return 1;
	}

	if (geteuid() != 0)
	{
		missingSetuidRoot();
		return 1;
	}

	g_originalUid = getuid();
	g_originalGid = getgid();

	setuid(0);
	setgid(0);

	prefix = getenv("DPREFIX");
	if (!prefix)
		prefix = defaultPrefixPath();
	if (!prefix)
		return 1;
	if (strlen(prefix) > 255)
	{
		fprintf(stderr, "Prefix path too long\n");
		return 1;
	}
	unsetenv("DPREFIX");
	getcwd(g_workingDirectory, sizeof(g_workingDirectory));

	if (!checkPrefixDir())
	{
		setupPrefix();
		g_fixPermissions = true;
	}
	checkPrefixOwner();

	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			{"help", 	no_argument, 0, 0},
			{"version", no_argument, 0, 0},
			{0, 		0, 			 0, 0}
		};
		int option_index = 0;

		c = getopt_long(argc, argv, "+", long_options, &option_index);

		if (c == -1)
		{
			break;
		}

		switch (c)
		{
			case 0:
			if (strcmp(long_options[option_index].name, "help") == 0)
			{
				showHelp(argv[0]);
				exit(EXIT_SUCCESS);
			}
			else if (strcmp(long_options[option_index].name, "version") == 0)
			{
				showVersion(argv[0]);
				exit(EXIT_SUCCESS);
			}
			break;
			case '?':
			break;
			default:
			abort();
		}
	}

	pidInit = getInitProcess();

	if (strcmp(argv[1], "shutdown") == 0)
	{
		if (pidInit == 0)
		{
			fprintf(stderr, "Osxie container is not running\n");
			return 1;
		}

		killOsxieContainer();
		return 0;
	}

	// If prefix's init is not running, start it up
	if (pidInit == 0)
	{
		char socketPath[4096];
		
		snprintf(socketPath, sizeof(socketPath), "%s"  SHELLSPAWN_SOCKPATH, prefix);
		
		unlink(socketPath);
		
		setupWorkdir();
		pidInit = spawnInitProcess();
		
		// Wait until shellspawn starts (check the container's view of the
		// socket, since that is the one we connect to). Also watch out for
		// the init process dying while we wait, which would mean the
		// container failed to boot.
		snprintf(socketPath, sizeof(socketPath), "/proc/%d/root%s"  SHELLSPAWN_SOCKPATH, pidInit, prefix);
		for (int i = 0; i < 30; i++)
		{
			if (access(socketPath, F_OK) == 0)
				break;

			if (kill(pidInit, 0) == -1)
			{
				fprintf(stderr, "The container init process (PID %d) died while the container was starting.\n", pidInit);
				fprintf(stderr, "See %s/private/var/log/dserver.log for details.\n", prefix);
				exit(1);
			}

			if (i == 5)
				fprintf(stderr, "Waiting for the container to start...\n");

			sleep(1);
		}

		if (access(socketPath, F_OK) != 0)
		{
			fprintf(stderr, "Timed out waiting for shellspawn in the container (%s).\n", socketPath);
			fprintf(stderr, "See %s/private/var/log/dserver.log for details.\n", prefix);
			exit(1);
		}
	}

	seteuid(g_originalUid);

	if (strcmp(argv[1], "shell") == 0)
	{
		// Spawn the shell
		if (argc > 2)
			spawnShell((const char**) &argv[2]);
		else
			spawnShell(NULL);
	}
	else
	{
		bool doExec = strcmp(argv[1], "exec") == 0;
		int argvIndex = doExec ? 2 : 1;

		if (doExec && argc <= 2)
		{
			printf("'exec' subcommand requires a binary to execute.\n");
			return 1;
		}

		char *fullPath;
		char *path = realpath(argv[argvIndex], NULL);

		if (path == NULL)
		{
			printf("'%s' is not a supported command or a file.\n", argv[argvIndex]);
			return 1;
		}

		fullPath = malloc(strlen(SYSTEM_ROOT) + strlen(path) + 1);
		strcpy(fullPath, SYSTEM_ROOT);
		strcat(fullPath, path);
		free(path);

		argv[argvIndex] = fullPath;

		if (doExec)
			spawnBinary(argv[argvIndex], (const char**) &argv[argvIndex]);
		else
			spawnShell((const char**) &argv[argvIndex]);
	}

	return 0;
}

static void pushShellspawnCommandData(int sockfd, shellspawn_cmd_type_t type, const void* data, size_t data_length)
{
	struct shellspawn_cmd* cmd;
	size_t length;

	length = sizeof(*cmd) + data_length;

	cmd = (struct shellspawn_cmd*) malloc(length);
	cmd->cmd = type;
	cmd->data_length = data_length;

	if (data != NULL)
		memcpy(cmd->data, data, data_length);

	if (write(sockfd, cmd, length) != length)
	{
		fprintf(stderr, "Error sending command to shellspawn: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	free(cmd);
}

static void pushShellspawnCommand(int sockfd, shellspawn_cmd_type_t type, const char* value)
{
	if (!value)
		pushShellspawnCommandData(sockfd, type, NULL, 0);
	else
		pushShellspawnCommandData(sockfd, type, value, strlen(value) + 1);
}

static void pushShellspawnCommandFDs(int sockfd, shellspawn_cmd_type_t type, const int fds[3])
{
	struct shellspawn_cmd cmd;
	char cmsgbuf[CMSG_SPACE(sizeof(int) * 3)];
	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *cmptr;

	cmd.cmd = type;
	cmd.data_length = 0;

	iov.iov_base = &cmd;

	memset(&msg, 0, sizeof(msg));
	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);

	iov.iov_base = &cmd;
	iov.iov_len = sizeof(cmd);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int) * 3);
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	memcpy(CMSG_DATA(cmptr), fds, sizeof(fds[0])*3);

	if (sendmsg(sockfd, &msg, 0) < 0)
	{
		fprintf(stderr, "Error sending command to shellspawn: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static int _shSockfd = -1;
static struct termios orig_termios;
static int pty_master;
static void signalHandler(int signo)
{
	// printf("Received signal %d\n", signo);

	// Forward window size changes
	if (signo == SIGWINCH && pty_master != -1)
	{
		struct winsize win;

		ioctl(0, TIOCGWINSZ, &win);
		ioctl(pty_master, TIOCSWINSZ, &win);
	}
	
	// Foreground process loopkup in shellspawn doesn't work
	// if we're not running in TTY mode, so shellspawn falls back
	// to forwarding signals to the Bash subprocess.
	// 
	// Hence we translate SIGINT to SIGTERM for user convenience,
	// because Bash will not terminate on SIGINT.
	bool shouldKillContainer = false;
	if (pty_master == -1 && signo == SIGINT) {
		signo = SIGTERM;
		shouldKillContainer = true;
	} else if (signo == SIGTERM) {
		shouldKillContainer = true;
	}

	pushShellspawnCommandData(_shSockfd, SHELLSPAWN_SIGNAL, &signo, sizeof(signo));

	// Also tear down the whole container so the app and its helper
	// processes don't linger as orphans/zombies after the user quits.
	if (shouldKillContainer)
		killOsxieContainer();
}

static void shellLoop(int sockfd, int master)
{
	struct sigaction sa;
	struct pollfd pfds[3];
	const int fdcount = (master != -1) ? 3 : 1; // do we do pty proxying?

	// Vars for signal handler
	_shSockfd = sockfd;
	pty_master = master;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = signalHandler;
	sigfillset(&sa.sa_mask);

	for (int i = 1; i < 32; i++)
		sigaction(i, &sa, NULL);

	pfds[2].fd = master;
	pfds[2].events = POLLIN;
	pfds[2].revents = 0;
	pfds[1].fd = STDIN_FILENO;
	pfds[1].events = POLLIN;
	pfds[1].revents = 0;
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN;
	pfds[0].revents = 0;

	if (master != -1)
		fcntl(master, F_SETFL, O_NONBLOCK);
	//fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	while (1)
	{
		char buf[4096];

		if (poll(pfds, fdcount, -1) < 0)
		{
			if (errno != EINTR)
			{
				perror("poll");
				break;
			}
		}

		if (pfds[2].revents & POLLIN)
		{
			int rd;
			do
			{
				rd = read(master, buf, sizeof(buf));
				if (rd > 0)
					write(STDOUT_FILENO, buf, rd);
			}
			while (rd == sizeof(buf));
		}

		if (pfds[1].revents & POLLIN)
		{
			int rd = 0;
			do
			{
				if (ioctl(STDIN_FILENO, FIONREAD, &rd) < 0) {
					perror("ioctl");
					exit(1);
				}
				if (rd > sizeof(buf)) {
					rd = sizeof(buf);
				}
				rd = read(STDIN_FILENO, buf, rd);
				if (rd > 0)
					write(master, buf, rd);
				else {
					perror("read");
					exit(1);
				}
			}
			while (rd == sizeof(buf));
		}

		if (pfds[0].revents & (POLLHUP | POLLIN))
		{
			int exitStatus;
			
			if (read(sockfd, &exitStatus, sizeof(int)) == sizeof(int))
			{
				if (exitStatus < 0)
				{
					// Negative exit statuses are errors reported by
					// shellspawn (the errno, negated). Show them like a
					// shell would.
					int err = -exitStatus;
					if (err == 0)
						err = EIO;
					fprintf(stderr, "osxie: cannot execute: %s\n", strerror(err));
					exit(err == ENOENT ? 127 : 126);
				}
				exit(exitStatus);
			}
			else
				exit(1);
		}
	}
}

static void restoreTermios(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

// Glibc openpty() fails for me on Debian, because grantpty() fails to chown() the pty node with error code EPERM.
// This is a more lenient version of openpty() that just works.
static int openpty_osxie(int* amaster, int* aslave, char* name_unused, const struct termios* tos, const struct winsize* wsz)
{
	const char* slave_name;

	*amaster = posix_openpt(O_RDWR);
	if (*amaster == -1)
		return -1;

	grantpt(*amaster);
	if (unlockpt(*amaster) < 0)
		return -1;

	slave_name = ptsname(*amaster);
	*aslave = open(slave_name, O_RDWR | O_NOCTTY);
	if (*aslave == -1)
		return -1;

	if (tos != NULL)
		tcsetattr(*amaster, TCSANOW, tos);
	if (wsz != NULL)
		ioctl(*amaster, TIOCSWINSZ, wsz);

	return 0;
}

static void setupPtys(int fds[3], int* master)
{
	struct winsize win;
	struct termios termios;
	bool tty = true;

	if (tcgetattr(STDIN_FILENO, &termios) < 0)
		tty = false;

	if (openpty_osxie(master, &fds[0], NULL, &termios, NULL) < 0)
	{
		perror("openpty");
		exit(1);
	}
	fds[2] = fds[1] = fds[0];

	if (tty)
	{
		orig_termios = termios;

		ioctl(0, TIOCGWINSZ, &win);

		termios.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
		termios.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
				INPCK | ISTRIP | IXON | PARMRK);
		termios.c_oflag &= ~OPOST;
		termios.c_cc[VMIN] = 1;
		termios.c_cc[VTIME] = 0;

		if (tcsetattr(STDIN_FILENO, TCSANOW, &termios) < 0)
		{
			perror("tcsetattr");
			exit(1);
		}
		ioctl(*master, TIOCSWINSZ, &win);

		atexit(restoreTermios);
	}
}

// Replace each quote character (') with the sequence '\''
// (copying the result to dest, if non-null)
// and return the length of the result.
static size_t escapeQuotes(char *dest, const char *src)
{
	size_t len = 0;

	for (; *src != 0; src++)
	{
		if (*src == '\'')
		{
			if (dest)
				memcpy(&dest[len], "'\\''", 4);
			len += 4;
		}
		else
		{
			if (dest)
				dest[len] = *src;
			len++;
		}
	}
	if (dest)
		dest[len] = 0;

	return len;
}

int connectToShellspawn(void)
{
	struct sockaddr_un addr;
	int sockfd;

	// Connect to the shellspawn daemon in the container.
	// shellspawn binds the socket inside the container's mount namespace,
	// where the prefix is covered by an overlayfs mount. Overlayfs exposes
	// a different inode for that socket than the plain file visible in the
	// host namespace, so connecting to the plain prefix path reaches a
	// stale socket with no listener (ECONNREFUSED). Resolving through
	// /proc/<init-pid>/root uses the container's mount namespace and
	// reaches the actual listening socket.
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "/proc/%d/root%s"  SHELLSPAWN_SOCKPATH, pidInit, prefix);

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		fprintf(stderr, "Error creating a unix domain socket: %s\n", strerror(errno));
		exit(1);
	}

	// Resolving /proc/<init-pid>/root applies a ptrace access check that
	// compares the caller's real uid against the container processes'. The
	// real uid is 0 here (we setuid(0) in main), but by this point the
	// effective uid is g_originalUid (seteuid in main), which has no
	// CAP_SYS_PTRACE, so the magic link resolves to EACCES. Raise the
	// effective uid back to 0 for the duration of the connect (allowed, as
	// the real and saved uids are 0).
	seteuid(0);

	// The socket file may exist while shellspawn is still binding or
	// while launchd is restarting it, so retry a few times and bail out
	// with a clear message if the container init dies in the meantime.
	// A failed connect() leaves an AF_UNIX stream socket in an undefined
	// state, so recreate it on every attempt.
	int connectResult = -1;
	for (int attempt = 0; attempt < 5; attempt++)
	{
		if (attempt > 0)
		{
			close(sockfd);
			sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
			if (sockfd == -1)
			{
				fprintf(stderr, "Error creating a unix domain socket: %s\n", strerror(errno));
				seteuid(g_originalUid);
				exit(1);
			}
		}

		if (kill(pidInit, 0) == -1)
		{
			fprintf(stderr, "The container init process (PID %d) died while connecting to shellspawn.\n", pidInit);
			seteuid(g_originalUid);
			exit(1);
		}

		connectResult = connect(sockfd, (struct sockaddr*) &addr, sizeof(addr));
		if (connectResult == 0)
			break;

		if (errno != ECONNREFUSED && errno != ENOENT && errno != EAGAIN)
			break;

		sleep(1);
	}
	seteuid(g_originalUid);

	if (connectResult == -1)
	{
		fprintf(stderr, "Error connecting to shellspawn in the container (%s): %s\n", addr.sun_path, strerror(errno));
		exit(1);
	}

	return sockfd;
}

void setupShellspawnEnv(int sockfd)
{
	static const char* skip_vars[] = {
		"PATH",
		"TMPDIR",
		"HOME",
	};

	char buffer2[4096];

	// Push environment variables
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, 
		"PATH=/usr/bin:"
		"/bin:"
		"/usr/sbin:"
		"/sbin:"
		"/usr/local/bin");
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, "TMPDIR=/private/tmp");

	const char* login = NULL;
	struct passwd* pw = getpwuid(geteuid());

	if (pw != NULL)
		login = pw->pw_name;

	if (!login)
		login = getlogin();
	if (!login)
	{
		fprintf(stderr, "Cannot determine your user name\n");
		exit(1);
	}

	snprintf(buffer2, sizeof(buffer2), "HOME=/Users/%s", login);
	pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, buffer2);

	for (char** var_ptr = environ; *var_ptr != NULL; ++var_ptr) {
		const char* var = *var_ptr;
		const char* equal = strchr(var, '=');
		size_t name_len = (equal != NULL) ? (size_t)(equal - var) : strlen(var);
		bool skip_it = false;

		for (size_t i = 0; i < sizeof(skip_vars) / sizeof(*skip_vars); ++i) {
			if (strlen(skip_vars[i]) == name_len && strncmp(var, skip_vars[i], name_len) == 0) {
				skip_it = true;
				break;
			}
		}

		if (skip_it) {
			continue;
		}

		pushShellspawnCommand(sockfd, SHELLSPAWN_SETENV, var);
	}
}

void setupWorkingDir(int sockfd)
{
	char buffer2[4096];
	snprintf(buffer2, sizeof(buffer2), SYSTEM_ROOT "%s", g_workingDirectory);
	pushShellspawnCommand(sockfd, SHELLSPAWN_CHDIR, buffer2);
}

void setupIDs(int sockfd)
{
	int ids[2] = { g_originalUid, g_originalGid };
	pushShellspawnCommandData(sockfd, SHELLSPAWN_SETUIDGID, ids, sizeof(ids));
}

void setupFDs(int fds[3], int* master)
{
	*master = -1;

	if (isatty(STDIN_FILENO))
		setupPtys(fds, master);
	else
		fds[0] = dup(STDIN_FILENO); // dup() because we close() after spawning
	
	if (*master == -1 || !isatty(STDOUT_FILENO))
		fds[1] = STDOUT_FILENO;
	if (*master == -1 || !isatty(STDERR_FILENO))
		fds[2] = STDERR_FILENO;
}

void spawnGo(int sockfd, int fds[3], int master)
{
	pushShellspawnCommandFDs(sockfd, SHELLSPAWN_GO, fds);
	close(fds[0]);

	shellLoop(sockfd, master);
	
	if (master != -1)
		close(master);
	close(sockfd);
}

void spawnShell(const char** argv)
{
	size_t total_len = 0;
	int count;
	int sockfd;
	char* buffer;
	int fds[3], master;

	if (argv != NULL)
	{
		for (count = 0; argv[count] != NULL; count++)
			total_len += escapeQuotes(NULL, argv[count]);

		buffer = malloc(total_len + count*3);

		char *to = buffer;
		for (int i = 0; argv[i] != NULL; i++)
		{
			if (to != buffer)
				to = stpcpy(to, " ");
			to = stpcpy(to, "'");
			to += escapeQuotes(to, argv[i]);
			to = stpcpy(to, "'");
		}
	}
	else
		buffer = NULL;

	sockfd = connectToShellspawn();

	setupShellspawnEnv(sockfd);

	// Push shell arguments
	if (buffer != NULL)
	{
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, "-c");
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, buffer);

		free(buffer);
	}

	setupWorkingDir(sockfd);
	setupIDs(sockfd);
	setupFDs(fds, &master);
	spawnGo(sockfd, fds, master);
}

void spawnBinary(const char* binary, const char** argv)
{
	int fds[3], master;
	int sockfd;

	sockfd = connectToShellspawn();
	setupShellspawnEnv(sockfd);

	pushShellspawnCommand(sockfd, SHELLSPAWN_SETEXEC, binary);

	for (; *argv != NULL; ++argv)
		pushShellspawnCommand(sockfd, SHELLSPAWN_ADDARG, *argv);

	setupWorkingDir(sockfd);
	setupIDs(sockfd);
	setupFDs(fds, &master);
	spawnGo(sockfd, fds, master);
}

void showHelp(const char* argv0)
{
	fprintf(stderr, "This is Osxie, translation layer for macOS software.\n\n");
	fprintf(stderr, "Copyright (C) 2012-2023 Lubos Dolezel\n\n");

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s <program-path> [arguments...]\n", argv0);
	fprintf(stderr, "\t%s shell [arguments...]\n", argv0);
	fprintf(stderr, "\t%s exec <program-path> [arguments...]\n", argv0);
	fprintf(stderr, "\t%s shutdown\n", argv0);
	fprintf(stderr, "\n");
	fprintf(stderr, "Environment variables:\n"
		"DPREFIX - specifies the location of Osxie prefix, defaults to ~/.osxie\n");
}

void showVersion(const char* argv0) {
	fprintf(stderr, "%s " GIT_BRANCH " @ " GIT_COMMIT_HASH "\n", argv0);
	fprintf(stderr, "Copyright (C) 2012-2023 Lubos Dolezel\n");
}

void missingSetuidRoot(void)
{
	char path[4096];
	int len;

	len = readlink("/proc/self/exe", path, sizeof(path)-1);
	if (len < 0)
		strcpy(path, "osxie");
	else
		path[len] = '\0';

	fprintf(stderr, "Sorry, the `%s' binary is not setuid root, which is mandatory.\n", path);
	fprintf(stderr, "Osxie needs this in order to create mount and PID namespaces and to perform mounts.\n");
}

pid_t spawnInitProcess(void)
{
	pid_t pid;
	int pipefd[2];
	char buffer[1];

	if (pipe(pipefd) == -1)
	{
		fprintf(stderr, "Cannot create a pipe for synchronization: %s\n", strerror(errno));
		exit(1);
	}

	if (unshare(CLONE_NEWUTS | CLONE_NEWIPC) != 0)
	{
		fprintf(stderr, "Cannot unshare UTS and IPC namespaces to create osxie-init: %s\n", strerror(errno));
		exit(1);
	}

	pid = fork();

	if (pid < 0)
	{
		fprintf(stderr, "Cannot fork() to create osxie-init: %s\n", strerror(errno));
		exit(1);
	}

	if (pid == 0)
	{
		// The child

		char uid_str[21];
		char gid_str[21];
		char pipefd_str[21];

		snprintf(uid_str, sizeof(uid_str), "%d", g_originalUid);
		snprintf(gid_str, sizeof(gid_str), "%d", g_originalGid);
		snprintf(pipefd_str, sizeof(pipefd_str), "%d", pipefd[1]);

		close(pipefd[0]);

		execl(INSTALL_PREFIX "/bin/osxieserver", "osxieserver", prefix, uid_str, gid_str, pipefd_str, g_fixPermissions ? "1" : "0", NULL);

		fprintf(stderr, "Failed to start osxieserver\n");
		exit(1);
	}

	// Wait for the child to drop UID/GIDs and unshare stuff
	close(pipefd[1]);

	// Fallback 1: Check pipe read with timeout using alarm()
	{
		struct sigaction sa;
		int saved_timeout = 0;
		
		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sigaction(SIGALRM, &sa, NULL);
		
		saved_timeout = alarm(5);  // Max 5 seconds to wait for child
		if (saved_timeout == 0) {
			saved_timeout = 1;  // We set it, will be cleared by alarm
		} else if (saved_timeout != 0) {
			alarm(saved_timeout);  // Resume previous pending alarm
		}

		int read_result = read(pipefd[0], buffer, 1);
		if (read_result < 0) {
			fprintf(stderr, "Failed to read from pipe (child may have failed): %s\n", strerror(errno));
			alarm(0);  // Cancel the alarm
			exit(1);
		} else if (read_result == 0) {
			fprintf(stderr, "Child exited without signaling readiness\n");
			alarm(0);
			exit(1);
		}

		alarm(0);  // Cancel the alarm
		close(pipefd[0]);
	}

	/*

	pid_t wstatus;
	if ((wstatus = waitpid(pid, NULL, WNOHANG)) != pid) {
		if (wstatus == 0 || wstatus < 0) {
			fprintf(stderr, "waitpid failed: child %d never started or already dead\n", pid);
			exit(1);
		} else {
			fprintf(stderr, "waitpid returned %d (unexpected)\n", wstatus);
		}
	}

	file = fopen(idmap, "w");
	if (file != NULL)
	{
		fprintf(file, "0 %d 1\n", g_originalUid); // all users map to our user on the outside
		fclose(file);
	}
	else
	{
		fprintf(stderr, "Cannot set uid_map for the init process: %s\n", strerror(errno));
	}

	snprintf(idmap, sizeof(idmap), "/proc/%d/gid_map", pid);

	file = fopen(idmap, "w");
	if (file != NULL)
	{
		fprintf(file, "0 %d 1\n", g_originalGid); // all groups map to our group on the outside
		fclose(file);
	}
	else
	{
		fprintf(stderr, "Cannot set gid_map for the init process: %s\n", strerror(errno));
	}
	*/

	{
		const char pidFile[] = "/.init.pid";
		char* pidPath;
		FILE *fp;

		pidPath = (char*) alloca(strlen(prefix) + sizeof(pidFile));
		strcpy(pidPath, prefix);
		strcat(pidPath, pidFile);

		seteuid(g_originalUid);
		setegid(g_originalGid);

		fp = fopen(pidPath, "w");

		seteuid(0);
		setegid(0);

		if (fp == NULL)
		{
			fprintf(stderr, "Cannot write out PID of the init process: %s\n", strerror(errno));
			return pid;
		}
		fprintf(fp, "%d", (int) pid);
		fclose(fp);
	}

	return pid;
}

char* defaultPrefixPath(void)
{
	const char defaultPath[] = "/.osxie";
	const char* home = getenv("HOME");
	char* buf;

	if (!home)
	{
		fprintf(stderr, "Cannot detect your home directory!\n");
		return NULL;
	}

	buf = (char*) malloc(strlen(home) + sizeof(defaultPath));
	strcpy(buf, home);
	strcat(buf, defaultPath);

	return buf;
}

void createDir(const char* path)
{
	struct stat st;

	if (stat(path, &st) == 0)
	{
		if (!S_ISDIR(st.st_mode))
		{
			fprintf(stderr, "%s already exists and is a file. Remove the file.\n", path);
			exit(1);
		}
	}
	else
	{
		if (errno == ENOENT)
		{
			if (mkdir(path, 0755) != 0)
			{
				fprintf(stderr, "Cannot create %s: %s\n", path, strerror(errno));
				exit(1);
			}
		}
		else
		{
			fprintf(stderr, "Cannot access %s: %s\n", path, strerror(errno));
			exit(1);
		}
	}
}

void setupWorkdir()
{
	char* workdir;
	const char suffix[] = ".workdir";
	size_t len;

	len = strlen(prefix);
	workdir = (char*) alloca(len + sizeof(suffix));
	strcpy(workdir, prefix);

	// Remove trailing /
	while (workdir[len-1] == '/')
		len--;
	workdir[len] = '\0';

	strcat(workdir, suffix);

	createDir(workdir);
}

int checkPrefixDir()
{
	struct stat st;
	char volumes_path[4096];

	if (stat(prefix, &st) == 0)
	{
		if (!S_ISDIR(st.st_mode))
		{
			fprintf(stderr, "%s is a file. Remove the file.\n", prefix);
			exit(1);
		}

		snprintf(volumes_path, sizeof(volumes_path), "%s/Volumes", prefix);
		if (stat(volumes_path, &st) != 0 || !S_ISDIR(st.st_mode))
			return 0; // directory exists but not initialized

		return 1; // OK
	}
	if (errno == ENOENT)
		return 0; // not found
	fprintf(stderr, "Cannot access %s: %s\n", prefix, strerror(errno));
	exit(1);
}

void setupPrefix()
{
	char path[4096];
	size_t plen;
	FILE* file;
	struct passwd* passwd_entry;
	
	const char* dirs[] = {
		"/Volumes",
		"/Applications",
		"/usr",
		"/usr/local",
		"/usr/local/share",
		"/private",
		"/private/var",
		"/private/var/log",
		"/private/var/db",
		"/private/etc",
		"/var",
		"/var/run",
		"/var/tmp",
		"/var/log"
	};

	fprintf(stderr, "Setting up a new Osxie prefix at %s\n", prefix);

	seteuid(g_originalUid);
	setegid(g_originalGid);

	createDir(prefix);
	strcpy(path, prefix);
	strcat(path, "/");
	plen = strlen(path);

	for (size_t i = 0; i < sizeof(dirs)/sizeof(dirs[0]); i++)
	{
		path[plen] = '\0';
		strcat(path, dirs[i]);
		createDir(path);
	}

	// create passwd, master.passwd, and group

	passwd_entry = getpwuid(g_originalUid);
	if (!passwd_entry) {
		fprintf(stderr, "Failed to find Linux /etc/passwd entry for current user\n");
		exit(1);
	}

	path[plen] = '\0';
	strcat(path, "/private/etc/passwd");
	file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "Failed to open /private/etc/passwd within the prefix\n");
		exit(1);
	}

	fprintf(file,
		"root:*:0:0:System Administrator:/var/root:/bin/sh\n"
		"%s:*:%d:%d:Osxie User:/Users/%s:/bin/bash\n",
		passwd_entry->pw_name,
		passwd_entry->pw_uid,
		passwd_entry->pw_gid,
		passwd_entry->pw_name
	);
	fclose(file);

	path[plen] = '\0';
	strcat(path, "/private/etc/master.passwd");
	file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "Failed to open /private/etc/master.passwd within the prefix\n");
		exit(1);
	}

	fprintf(file,
		"root:*:0:0::0:0:System Administrator:/var/root:/bin/sh\n"
		"%s:*:%d:%d::0:0:Osxie User:/Users/%s:/bin/bash\n",
		passwd_entry->pw_name,
		passwd_entry->pw_uid,
		passwd_entry->pw_gid,
		passwd_entry->pw_name
	);
	fclose(file);

	path[plen] = '\0';
	strcat(path, "/private/etc/group");
	file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "Failed to open /private/etc/group within the prefix\n");
		exit(1);
	}

	fprintf(file,
		"wheel:*:0:root,%s\n"
		"%s:*:%d:%s\n",
		passwd_entry->pw_name,
		passwd_entry->pw_name,
		passwd_entry->pw_gid,
		passwd_entry->pw_name
	);
	fclose(file);
	
	seteuid(0);
	setegid(0);
}

pid_t getInitProcess()
{
	const char pidFile[] = "/.init.pid";
	char* pidPath;
	pid_t pid;
	int pid_i;
	FILE *fp;
	char procBuf[100];
	char *exeBuf, *statusBuf;
	int uidMatch = 0, gidMatch = 0;

	pidPath = (char*) alloca(strlen(prefix) + sizeof(pidFile));
	strcpy(pidPath, prefix);
	strcat(pidPath, pidFile);

	fp = fopen(pidPath, "r");
	if (fp == NULL)
		return 0;

	if (fscanf(fp, "%d", &pid_i) != 1)
	{
		fclose(fp);
		unlink(pidPath);
		return 0;
	}
	fclose(fp);
	pid = (pid_t) pid_i;

	// Does the process exist?
	if (kill(pid, 0) == -1)
	{
		unlink(pidPath);
		return 0;
	}

	// Is it actually an init process?
	snprintf(procBuf, sizeof(procBuf), "/proc/%d/comm", pid);
	fp = fopen(procBuf, "r");
	if (fp == NULL)
	{
		unlink(pidPath);
		return 0;
	}

	if (fscanf(fp, "%ms", &exeBuf) != 1)
	{
		fclose(fp);
		unlink(pidPath);
		return 0;
	}
	fclose(fp);

	if (strcmp(exeBuf, "osxieserver") != 0)
	{
		unlink(pidPath);
		return 0;
	}
	free(exeBuf);

	// Is it owned by the current user?
	if (g_originalUid != 0)
	{
		snprintf(procBuf, sizeof(procBuf), "/proc/%d/status", pid);
		fp = fopen(procBuf, "r");
		if (fp == NULL)
		{
			unlink(pidPath);
			return 0;
		}

		while (1)
		{
			statusBuf = NULL;
			size_t len;
			if (getline(&statusBuf, &len, fp) == -1)
				break;
			int rid, eid, sid, fid;
			if (sscanf(statusBuf, "Uid: %d %d %d %d", &rid, &eid, &sid, &fid) == 4)
			{
				uidMatch = 1;
				uidMatch &= rid == g_originalUid;
				uidMatch &= eid == g_originalUid;
				uidMatch &= sid == g_originalUid;
				uidMatch &= fid == g_originalUid;
			}
			if (sscanf(statusBuf, "Gid: %d %d %d %d", &rid, &eid, &sid, &fid) == 4)
			{
				gidMatch = 1;
				gidMatch &= rid == g_originalGid;
				gidMatch &= eid == g_originalGid;
				gidMatch &= sid == g_originalGid;
				gidMatch &= fid == g_originalGid;
			}
			free(statusBuf);
		}
		fclose(fp);

		if (!uidMatch || !gidMatch)
		{
			unlink(pidPath);
			return 0;
		}
	}

	return pid;
}

void checkPrefixOwner()
{
	struct stat st;

	if (stat(prefix, &st) == 0)
	{
		if (g_originalUid != 0 && st.st_uid != g_originalUid)
		{
			fprintf(stderr, "You do not own the prefix directory.\n");
			exit(1);
		}
	}
	else if (errno == EACCES)
	{
		fprintf(stderr, "You do not own the prefix directory.\n");
		exit(1);
	}
}
