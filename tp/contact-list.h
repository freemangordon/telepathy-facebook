/*
 * contact-list.h
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
#ifndef FB_CONTACT_LIST_H
#define FB_CONTACT_LIST_H

#include <glib-object.h>

#include <telepathy-glib/base-channel.h>

#include "facebook-api.h"

#include "connection.h"

G_BEGIN_DECLS

typedef struct _FbContactList FbContactList;
typedef struct _FbContactListClass
  FbContactListClass;

GType
fb_contact_list_get_type (void);

/* TYPE MACROS */
#define FB_TYPE_CONTACT_LIST (fb_contact_list_get_type())
#define FB_CONTACT_LIST(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FB_TYPE_CONTACT_LIST, \
                              FbContactList))
#define FB_CONTACT_LIST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FB_TYPE_CONTACT_LIST, \
                           FbContactListClass))
#define TP_IS_FB_CONTACT_LIST(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FB_TYPE_CONTACT_LIST))
#define TP_IS_FB_CONTACT_LIST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FB_TYPE_CONTACT_LIST))
#define FB_CONTACT_LIST_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FB_TYPE_CONTACT_LIST, \
                             FbContactListClass))

FbContactList *
fb_contact_list_new(TpBaseConnection *connection);

void
fb_contact_list_fb_contacts_changed(FbContactList *self,
                                    FbApi *api, GSList *users);

FbContact *
fb_contact_list_get_user(FbContactList *self, TpHandle handle);

TpHandle
fb_contact_list_ensure_handle(FbContactList *self, FbId fb_uid);

#endif // FB_CONTACT_LIST_H
