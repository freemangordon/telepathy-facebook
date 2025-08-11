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

#define FB_DEBUG_FLAG FB_DEBUG_AVATAR

#include "avatars.h"
#include "connection.h"
#include "contact-list.h"
#include "debug.h"

struct avatar_request_queue
{
  FbHttp *http;
  GQueue *queue;
  FbConnection *conn;
};

static gboolean
idle_send_req(gpointer user_data)
{
  fb_http_request_send(user_data);

  return G_SOURCE_REMOVE;
}

static void
avatar_cb(FbHttpRequest *req, gpointer user_data)
{
  struct avatar_request_queue *arq = (struct avatar_request_queue *)user_data;
  FbContact *c = g_queue_pop_tail(arq->queue);
  gboolean connected =
    tp_base_connection_get_status(
      TP_BASE_CONNECTION(arq->conn)) == TP_CONNECTION_STATUS_CONNECTED;

  if (connected)
  {
    gint code;

    fb_http_request_get_status(req, &code);

    if (code == 200)
    {
      gsize icon_size;
      const gchar *icon_data = fb_http_request_get_data(req, &icon_size);
      FbContactList *contact_list = fb_connection_get_contact_list(arq->conn);

      if (icon_data)
      {
        GArray *avatar = g_array_sized_new(FALSE, FALSE,
                                           sizeof(gchar), icon_size);
        TpHandle handle = fb_contact_list_ensure_handle(contact_list, c->uid);

        g_array_append_vals(avatar, icon_data, icon_size);
        tp_svc_connection_interface_avatars_emit_avatar_retrieved(
          arq->conn, handle, c->avatar_token, avatar, "image/jpeg");
        g_array_free(avatar, TRUE);
      }
    }
  }

  if (g_queue_is_empty(arq->queue) || !connected)
  {
    g_object_unref(arq->http);
    g_queue_free(arq->queue);
    g_object_unref(arq->conn);
    g_slice_free(struct avatar_request_queue, arq);
  }
  else
  {
    c = g_queue_peek_tail(arq->queue);
    req = fb_http_request_new(arq->http, c->icon, FALSE,
                              avatar_cb, arq);
    g_idle_add_full(G_PRIORITY_LOW, idle_send_req, req, NULL);
  }
}

static void
fb_connection_avatars_request_avatars(TpSvcConnectionInterfaceAvatars *self,
                                      const GArray *contacts,
                                      DBusGMethodInvocation *context)
{
  FbConnection *conn = FB_CONNECTION(self);
  TpBaseConnection *base = TP_BASE_CONNECTION(conn);
  FbContactList *contact_list = fb_connection_get_contact_list(conn);
  TpHandleRepoIface *contact_repo =
    tp_base_connection_get_handles(base, TP_HANDLE_TYPE_CONTACT);
  GError *error = NULL;

  guint i;

  TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED(base, context);

  if (!tp_handles_are_valid(contact_repo, contacts, FALSE, &error))
  {
    dbus_g_method_return_error(context, error);
    g_error_free(error);
    return;
  }

  if (contacts->len)
  {
    struct avatar_request_queue *arq = g_new(struct avatar_request_queue, 1);

    arq->conn = g_object_ref(conn);
    arq->http = fb_http_new(FB_API_AGENT);
    arq->queue = g_queue_new();

    for (i = 0; i < contacts->len; i++)
    {
      TpHandle handle = g_array_index(contacts, TpHandle, i);
      FbContact *c = fb_contact_list_get_user(contact_list, handle);

      if (c->icon)
        g_queue_push_head(arq->queue, c);
    }

    FbContact *c = g_queue_peek_tail(arq->queue);
    FbHttpRequest *req = fb_http_request_new(arq->http, c->icon, FALSE,
                                             avatar_cb, arq);

    g_idle_add_full(G_PRIORITY_LOW, idle_send_req, req, NULL);
  }

  tp_svc_connection_interface_avatars_return_from_request_avatars(context);
}

#if 0
static void
fb_connection_avatars_get_known_avatar_tokens(
  TpSvcConnectionInterfaceAvatars *iface, const GArray *contacts,
  DBusGMethodInvocation *context)
{
  FbConnection *self = FB_CONNECTION(iface);
  TpBaseConnection *base = TP_BASE_CONNECTION(iface);
  FbContactList *contact_list = fb_connection_get_contact_list(self);
  TpHandleRepoIface *contact_repo =
    tp_base_connection_get_handles(base, TP_HANDLE_TYPE_CONTACT);

  g_autoptr(GHashTable) tokens =
    g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
  GError *error = NULL;

  TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED(base, context);

  if (!tp_handles_are_valid(contact_repo, contacts, FALSE, &error))
  {
    dbus_g_method_return_error(context, error);
    g_error_free(error);
    return;
  }

  for (guint i = 0; i < contacts->len; i++)
  {
    TpHandle handle = g_array_index(contacts, TpHandle, i);
    FbContact *c = fb_contact_list_get_user(contact_list, handle);

    if (c && c->avatar_token)
    {
      g_hash_table_insert(tokens, GUINT_TO_POINTER(handle),
                          g_strdup(c->avatar_token));
    }
  }

  tp_svc_connection_interface_avatars_return_from_get_known_avatar_tokens(
    context, tokens);

  g_hash_table_unref(tokens);
}

static void
fb_connection_avatars_get_avatar_tokens(
  TpSvcConnectionInterfaceAvatars *avatars, const GArray *contacts,
  DBusGMethodInvocation *context)
{
  FbConnection *self = FB_CONNECTION(avatars);
  TpBaseConnection *base = TP_BASE_CONNECTION(avatars);
  TpHandleRepoIface *contact_repo =
    tp_base_connection_get_handles(base, TP_HANDLE_TYPE_CONTACT);
  FbContactList *contact_list = fb_connection_get_contact_list(self);
  GError *error = NULL;
  GHashTable *result;
  guint i;

  TP_BASE_CONNECTION_ERROR_IF_NOT_CONNECTED(base, context);

  if (!tp_handles_are_valid(contact_repo, contacts, FALSE, &error))
  {
    dbus_g_method_return_error(context, error);
    g_error_free(error);
    return;
  }

  result = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);

  for (i = 0; i < contacts->len; i++)
  {
    TpHandle handle = g_array_index(contacts, TpHandle, i);
    FbContact *c = fb_contact_list_get_user(contact_list, handle);

    if (c->avatar_token)
    {
      tp_svc_connection_interface_avatars_emit_avatar_updated(
        self, handle, c->avatar_token);
    }

    g_hash_table_insert(result, GUINT_TO_POINTER(handle), c->avatar_token);
  }

  tp_svc_connection_interface_avatars_return_from_get_known_avatar_tokens(
    context, result);
  g_hash_table_unref(result);
}
#endif

void
fb_connection_avatars_iface_init (gpointer g_iface, gpointer iface_data)
{
  TpSvcConnectionInterfaceAvatarsClass *klass =
    (TpSvcConnectionInterfaceAvatarsClass *)g_iface;

#define IMPLEMENT(x) \
  tp_svc_connection_interface_avatars_implement_ ## x( \
    klass, fb_connection_avatars_ ## x)
//    IMPLEMENT(get_avatar_requirements);
//    IMPLEMENT(get_avatar_tokens);
//    IMPLEMENT(get_known_avatar_tokens);
  IMPLEMENT(request_avatars);
//    IMPLEMENT(set_avatar);
//    IMPLEMENT(clear_avatar);
#undef IMPLEMENT
}

static void
fb_connection_avatars_fill_contact_attributes(GObject *object,
                                              const GArray *contacts,
                                              GHashTable *attributes_hash)
{
  FbConnection *self = FB_CONNECTION(object);
  FbContactList *contact_list = fb_connection_get_contact_list(self);

  for (guint i = 0; i < contacts->len; i++)
  {
    TpHandle handle = g_array_index(contacts, TpHandle, i);
    const gchar *token;

    token = fb_contact_list_get_user(contact_list, handle)->avatar_token;

    if (!token)
      continue;

    tp_contacts_mixin_set_contact_attribute(
      attributes_hash, handle, TP_TOKEN_CONNECTION_INTERFACE_AVATARS_TOKEN,
      tp_g_value_slice_new_string(token));
  }
}

void
fb_connection_avatars_init(GObject *object)
{
  tp_contacts_mixin_add_contact_attributes_iface(
        object, TP_IFACE_CONNECTION_INTERFACE_AVATARS,
        fb_connection_avatars_fill_contact_attributes);
}
