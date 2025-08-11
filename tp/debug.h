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
#ifndef __FB_DEBUG_H__
#define __FB_DEBUG_H__

#include <glib.h>

typedef enum {
  FB_DEBUG_CONNECTION = (1 << 0),
  FB_DEBUG_ACCOUNT_VERIFY = (1 << 1),
  FB_DEBUG_AVATAR = (1 << 2),
} FbDebugFlags;

void fb_debug_init (void);
void fb_debug(FbDebugFlags flag, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

void fb_debug_free (void);

#endif

#ifdef FB_DEBUG_FLAG

#undef FB_DEBUG
#define FB_DEBUG(format, ...) \
  fb_debug(FB_DEBUG_FLAG, "%s: " format, G_STRFUNC, ##__VA_ARGS__)

#endif /* __FB_DEBUG_H__ */
