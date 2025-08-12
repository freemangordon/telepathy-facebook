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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct account_t account_t;
struct groupchat;

int bool2int(char *value);

typedef struct conf
{
    char *iface_in;
    char *iface_out;
    char *port;
    char *cafile;
} conf_t;

typedef struct global
{
    conf_t *conf;
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
