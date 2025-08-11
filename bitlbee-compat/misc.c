/*
 * misc.c
 *
 * Copyright (C) 2025 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 *
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

int is_bool(char *value)
{
    if (*value == 0) {
        return 0;
    }

    if ((g_strcasecmp(value,
                      "true") == 0) || (g_strcasecmp(value, "yes") == 0) || (g_strcasecmp(value, "on") == 0)) {
        return 1;
    }
    if ((g_strcasecmp(value,
                      "false") == 0) || (g_strcasecmp(value, "no") == 0) || (g_strcasecmp(value, "off") == 0)) {
        return 1;
    }

    while (*value) {
        if (!g_ascii_isdigit(*value)) {
            return 0;
        } else {
            value++;
        }
    }

    return 1;
}

char *get_rfc822_header(const char *text, const char *header, int len)
{
	int hlen = strlen(header), i;
	const char *ret;

	if (text == NULL) {
		return NULL;
	}

	if (len == 0) {
		len = strlen(text);
	}

	i = 0;
	while ((i + hlen) < len) {
		/* Maybe this is a bit over-commented, but I just hate this part... */
		if (g_strncasecmp(text + i, header, hlen) == 0) {
			/* Skip to the (probable) end of the header */
			i += hlen;

			/* Find the first non-[: \t] character */
			while (i < len && (text[i] == ':' || text[i] == ' ' || text[i] == '\t')) {
				i++;
			}

			/* Make sure we're still inside the string */
			if (i >= len) {
				return(NULL);
			}

			/* Save the position */
			ret = text + i;

			/* Search for the end of this line */
			while (i < len && text[i] != '\r' && text[i] != '\n') {
				i++;
			}

			/* Copy the found data */
			return(g_strndup(ret, text + i - ret));
		}

		/* This wasn't the header we were looking for, skip to the next line. */
		while (i < len && (text[i] != '\r' && text[i] != '\n')) {
			i++;
		}
		while (i < len && (text[i] == '\r' || text[i] == '\n')) {
			i++;
		}

		/* End of headers? */
		if ((i >= 4 && strncmp(text + i - 4, "\r\n\r\n", 4) == 0) ||
		    (i >= 2 && (strncmp(text + i - 2, "\n\n", 2) == 0 ||
				strncmp(text + i - 2, "\r\r", 2) == 0))) {
			break;
		}
	}

	return NULL;
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
