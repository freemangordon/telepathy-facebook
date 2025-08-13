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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dbus/dbus-glib.h>

#define FB_DEBUG_FLAG FB_DEBUG_CONNECTION

#include "connection.h"
#include "contact-list.h"
#include "presence.h"

#include "debug.h"

/* revisit FB API */
static const TpPresenceStatusSpec statuses[] =
{
  { "away", TP_CONNECTION_PRESENCE_TYPE_AWAY, FALSE, NULL, NULL, NULL },
  { "available", TP_CONNECTION_PRESENCE_TYPE_AVAILABLE, TRUE, NULL, NULL,
    NULL },
  { "unknown", TP_CONNECTION_PRESENCE_TYPE_UNKNOWN, TRUE, NULL, NULL, NULL },
/*  {"hidden", TP_CONNECTION_PRESENCE_TYPE_HIDDEN, TRUE, NULL, NULL, NULL},*/
  { NULL, TP_CONNECTION_PRESENCE_TYPE_UNSET, FALSE, NULL, NULL, NULL }
};

static GHashTable *
_get_contact_statuses(GObject *obj, const GArray *contacts,
                      GError **error)
{
  TpBaseConnection *base_conn = TP_BASE_CONNECTION(obj);
  TpHandleRepoIface *handle_repo =
    tp_base_connection_get_handles(base_conn, TP_HANDLE_TYPE_CONTACT);
  FbConnection *conn = FB_CONNECTION(obj);
  FbContactList *contact_list = fb_connection_get_contact_list(conn);
  GHashTable *status_table = g_hash_table_new_full(
    g_direct_hash, g_direct_equal,
    NULL, (GDestroyNotify)tp_presence_status_free);

  for (int i = 0; i < contacts->len; i++)
  {
    TpHandle handle = g_array_index(contacts, TpHandle, i);

    g_assert(tp_handle_is_valid(handle_repo, handle, NULL));

    TpPresenceStatus *status;

    if (G_UNLIKELY(handle == tp_base_connection_get_self_handle(base_conn)))
      status = tp_presence_status_new(1, NULL);
    else
    {
      FbContact *c = fb_contact_list_get_user(contact_list, handle);

      if (c->fs == FB_API_FRIENDSHIP_STATUS_ARE_FRIENDS)
        status = tp_presence_status_new(c->active, NULL);
      else
        status = tp_presence_status_new(2, NULL);
    }

    g_hash_table_insert(status_table, GUINT_TO_POINTER(handle), status);
  }

  return status_table;
}

static gboolean
_set_own_status (GObject *obj,
                 const TpPresenceStatus *status,
                 GError **error)
{
  return TRUE;
}

void
fb_connection_presence_class_init(GObjectClass *object_class)
{
  tp_presence_mixin_class_init(
    object_class,
    G_STRUCT_OFFSET(FbConnectionClass, presence_class),
    NULL, _get_contact_statuses, _set_own_status, statuses);

  tp_presence_mixin_simple_presence_init_dbus_properties(object_class);
}

void
fb_connection_presence_init(GObject *object)
{
  tp_presence_mixin_init(object, G_STRUCT_OFFSET(FbConnection, presence));
  tp_presence_mixin_simple_presence_register_with_contacts_mixin(object);
}

void
fb_contact_list_fb_presences_changed(FbConnection *self, GSList *presences)
{
  TpBaseConnection *base_conn = TP_BASE_CONNECTION(self);
  TpHandleRepoIface *contact_repo =
    tp_base_connection_get_handles(base_conn, TP_HANDLE_TYPE_CONTACT);
  FbContactList *contact_list = fb_connection_get_contact_list(self);
  GHashTable *status_table = g_hash_table_new_full(
    g_direct_hash, g_direct_equal,
    NULL, (GDestroyNotify)tp_presence_status_free);
  gchar uid[FB_ID_STRMAX];

  for (GSList *l = presences; l != NULL; l = l->next)
  {
    FbApiPresence *pres = l->data;

    FB_ID_TO_STR(pres->uid, uid);

    TpHandle handle = tp_handle_ensure(contact_repo, uid, NULL, NULL);

    if (handle)
    {
      FbContact *c = fb_contact_list_get_user(contact_list, handle);

      if (c->active != pres->active)
      {
        FB_DEBUG("%s presence changed (%d->%d)", uid, c->active, pres->active);

        c->active = pres->active;

        g_hash_table_insert(status_table, GUINT_TO_POINTER(handle),
                            tp_presence_status_new(c->active, NULL));
      }
    }
  }

  tp_presence_mixin_emit_presence_update(G_OBJECT(self), status_table);
  g_hash_table_unref(status_table);
}
