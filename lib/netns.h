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

#ifndef __NETNS_H
#define __NETNS_H

extern int default_nsfd;

void init_netns(void);

int switch_ns(int nsfd, sigset_t *oldmask);
void restore_ns(sigset_t *oldmask);

int open_ns(int nsfd, const char *pathname, int flags);
int socket_ns(int nsfd, int domain, int type, int protocol);
int get_nsfd(const char *name);

#endif
