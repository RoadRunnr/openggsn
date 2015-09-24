/*
 * NETNS functions.
 * Copyright (C) 2014, 2015 Travelping GmbH
 *
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 *
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <fcntl.h>
#include <errno.h>

#include "netns.h"

#define NETNS_PATH "/var/run/netns"

int default_nsfd;

int switch_ns(int nsfd, sigset_t *oldmask)
{
	sigset_t intmask;

	sigfillset(&intmask);
	sigprocmask(SIG_BLOCK, &intmask, oldmask);

	return setns(nsfd, CLONE_NEWNET);
}

void restore_ns(sigset_t *oldmask)
{
	setns(default_nsfd, CLONE_NEWNET);

	sigprocmask(SIG_SETMASK, oldmask, NULL);
}

int open_ns(int nsfd, const char *pathname, int flags)
{
	sigset_t intmask, oldmask;
	int fd;
	int errsv;

	sigfillset(&intmask);
	sigprocmask(SIG_BLOCK, &intmask, &oldmask);

	setns(nsfd, CLONE_NEWNET);
	fd = open(pathname, flags);
	errsv = errno;
	setns(default_nsfd, CLONE_NEWNET);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	errno = errsv;
	return fd;
}

int socket_ns(int nsfd, int domain, int type, int protocol)
{
	sigset_t intmask, oldmask;
	int sk;
	int errsv;

	sigfillset(&intmask);
	sigprocmask(SIG_BLOCK, &intmask, &oldmask);

	setns(nsfd, CLONE_NEWNET);
	sk = socket(domain, type, protocol);
	errsv = errno;
	setns(default_nsfd, CLONE_NEWNET);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	errno = errsv;
	return sk;
}

void init_netns()
{
	if ((default_nsfd = open("/proc/self/ns/net", O_RDONLY)) < 0) {
		perror("init_netns");
		exit(EXIT_FAILURE);
	}
}

int get_nsfd(const char *name)
{
	int r;
	sigset_t intmask, oldmask;
	char path[MAXPATHLEN] = NETNS_PATH;

	r = mkdir(path, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	if (r < 0 && errno != EEXIST)
		return r;

	snprintf(path, sizeof(path), "%s/%s", NETNS_PATH, name);
	r = open(path, O_RDONLY|O_CREAT|O_EXCL, 0);
	if (r < 0) {
		if (errno == EEXIST)
			return open(path, O_RDONLY);

		return r;
	}
	close(r);

	sigfillset(&intmask);
	sigprocmask(SIG_BLOCK, &intmask, &oldmask);

	unshare(CLONE_NEWNET);
	mount("/proc/self/ns/net", path, "none", MS_BIND, NULL);

	setns(default_nsfd, CLONE_NEWNET);

	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	return open(path, O_RDONLY);
}