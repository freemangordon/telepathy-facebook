/*
 * This file is part of telepathy-facebook
 *
 * Copyright (C) 2025 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <stdio.h>

#include "ssl_client.h"
#include "sock.h"

int
bool2int(char *value)
{
    int i;

    if (!g_ascii_strncasecmp(value, "true", 4) ||
        !g_ascii_strncasecmp(value, "yes", 3) ||
        !g_ascii_strncasecmp(value, "on", 2))
    {
        return 1;
    }

    if (!g_ascii_strncasecmp(value, "false", 5) ||
        !g_ascii_strncasecmp(value, "no", 2) ||
        !g_ascii_strncasecmp(value, "off", 3)) {
        return 0;
    }

    if (sscanf(value, "%d", &i) == 1) {
        return i;
    }

    return 0;
}


gboolean ssl_sockerr_again(void *ssl)
{
	if (ssl) {
		return ssl_errno == SSL_AGAIN;
	} else {
		return sockerr_again();
	}
}

/* A wrapper for /dev/urandom.
 * If /dev/urandom is not present or not usable, it calls abort()
 * to prevent bitlbee from working without a decent entropy source */
void random_bytes(unsigned char *buf, int count)
{
	int fd;

	if (((fd = open("/dev/urandom", O_RDONLY)) == -1) ||
	    (read(fd, buf, count) == -1)) {
		fprintf(stderr, "/dev/urandom not present - aborting");
		abort();
	}

	close(fd);
}
