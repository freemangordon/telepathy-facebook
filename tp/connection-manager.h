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
#ifndef __FB_CONNECTION_MANAGER_H__
#define __FB_CONNECTION_MANAGER_H__

#include <glib-object.h>
#include <telepathy-glib/telepathy-glib.h>

G_BEGIN_DECLS

typedef struct _FbConnectionManager FbConnectionManager;
typedef struct _FbConnectionManagerClass FbConnectionManagerClass;

struct _FbConnectionManagerClass
{
  TpBaseConnectionManagerClass parent_class;
};

struct _FbConnectionManager
{
  TpBaseConnection parent;
};

GType fb_connection_manager_get_type(void);

/* TYPE MACROS */
#define FB_TYPE_CONNECTION_MANAGER \
  (fb_connection_manager_get_type())
#define FB_CONNECTION_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FB_TYPE_CONNECTION_MANAGER, FbConnectionManager))
#define FB_CONNECTION_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FB_TYPE_CONNECTION_MANAGER, FbConnectionManagerClass))
#define FB_IS_CONNECTION_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FB_TYPE_CONNECTION_MANAGER))
#define FB_IS_CONNECTION_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FB_TYPE_CONNECTION_MANAGER))
#define FB_CONNECTION_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FB_TYPE_CONNECTION_MANAGER, FbConnectionManagerClass))

G_END_DECLS

#endif /* __FB_CONNECTION_MANAGER_H__*/
