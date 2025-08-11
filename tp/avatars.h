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

#ifndef __FB_CONNECTION_AVATARS_H__
#define __FB_CONNECTION_AVATARS_H__

#include <glib-object.h>
#include <telepathy-glib/telepathy-glib.h>

#include "facebook-api.h"

G_BEGIN_DECLS

void
fb_connection_avatars_init(GObject *object);

void
fb_connection_avatars_iface_init (gpointer g_iface, gpointer iface_data);

G_END_DECLS

#endif /* __FB_CONNECTION_AVATARS_H__ */
