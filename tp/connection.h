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
#ifndef __FB_CONNECTION_H__
#define __FB_CONNECTION_H__

#include <glib-object.h>
#include <telepathy-glib/telepathy-glib.h>

#include "facebook-api.h"

G_BEGIN_DECLS

typedef struct _FbConnection FbConnection;
typedef struct _FbConnectionClass FbConnectionClass;

struct _FbConnectionClass
{
  TpBaseConnectionClass parent_class;
  TpContactsMixinClass contacts_class;
  TpPresenceMixinClass presence_class;

};

struct _FbConnection
{
  TpBaseConnection parent;
  TpContactsMixin contacts;
  TpPresenceMixin presence;
};

struct _FbContact
{
  FbId uid;
  gchar *name;
  gchar *icon;
  gchar *avatar_token;
  FbApiFriendshipStatus fs;
  gboolean active;
};

typedef struct _FbContact FbContact;

typedef struct _FbContactList FbContactList;

GType
fb_connection_get_type(void);

/* TYPE MACROS */
#define FB_TYPE_CONNECTION \
  (fb_connection_get_type())
#define FB_CONNECTION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FB_TYPE_CONNECTION, \
                              FbConnection))
#define FB_CONNECTION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FB_TYPE_CONNECTION, \
                           FbConnectionClass))
#define FB_IS_CONNECTION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FB_TYPE_CONNECTION))
#define FB_IS_CONNECTION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FB_TYPE_CONNECTION))
#define FB_CONNECTION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FB_TYPE_CONNECTION, \
                             FbConnectionClass))

void
fb_connection_send(FbConnection *conn, const gchar *msg);

FbContactList *
fb_connection_get_contact_list(FbConnection *self);

const gchar *const *
fb_connection_get_implemented_interfaces(void);

G_END_DECLS

#endif /* #ifndef __FB_CONNECTION_H__*/
