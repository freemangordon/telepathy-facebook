/*
 * bitlbee.h
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
#ifndef BITLBEE_H
#define BITLBEE_H

#include <glib.h>

#include <string.h>
#include <stdio.h>

#ifndef G_MODULE_EXPORT
#define G_MODULE_EXPORT
#endif

#define MAX_STRING 511
#define PASSWORD_PENDING "\r\rchangeme\r\r"

#define PACKAGE "TelepathyFacebook"
#define BITLBEE_VERSION "3.6"
#define BITLBEE_VER(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#define BITLBEE_VERSION_CODE BITLBEE_VER(3, 6, 0)
#define BITLBEE_ABI_VERSION_CODE 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct account_t account_t;
struct groupchat;

#include "misc.h"

typedef struct conf
{
    char *iface_in, *iface_out;
    char *port;
    char *cafile;
} conf_t;

typedef struct global {
    /* In forked mode, child processes store the fd of the IPC socket here. */
    int listen_socket;
    gint listen_watch_source_id;
    struct help *help;
    char *conf_file;
    conf_t *conf;
    GList *storage; /* The first backend in the list will be used for saving */
    GList *auth;    /* Authentication backends */
    char *helpfile;
    int restart;
} global_t;

extern global_t global;

void random_bytes(unsigned char *buf, int count);
gboolean ssl_sockerr_again(void *ssl);

/* g_memdup() deprecated as of glib 2.68.0 */
#ifndef GLIB_VERSION_2_68
#define g_memdup2 g_memdup
#endif

#ifdef __cplusplus
}
#endif

#endif // BITLBEE_H
