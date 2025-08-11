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

#define FB_DEBUG_FLAG FB_DEBUG_ACCOUNT_VERIFY

#include <telepathy-glib/telepathy-glib.h>

#include "account-verify-channel.h"
#include "account-verify-manager.h"

#include "debug.h"

struct _FbAccountVerifyManagerClass
{
  GObjectClass parent_class;
  /*<private>*/
};

struct _FbAccountVerifyManager
{
  /*<private>*/
  GObject parent;
};

struct _FbAccountVerifyManagerPrivate
{
  TpBaseConnection *conn;
  FbAccountVerifyChannel *channel;
  gboolean dispose_has_run;
};

typedef struct _FbAccountVerifyManagerPrivate
  FbAccountVerifyManagerPrivate;

#define PRIVATE(o) \
  ((FbAccountVerifyManagerPrivate *) \
   fb_account_verify_manager_get_instance_private( \
     FB_ACCOUNT_VERIFY_MANAGER(o)))

static void
channel_manager_iface_init(TpChannelManagerIface *iface);

G_DEFINE_TYPE_WITH_CODE(
  FbAccountVerifyManager, fb_account_verify_manager, G_TYPE_OBJECT,
  G_IMPLEMENT_INTERFACE(TP_TYPE_CHANNEL_MANAGER, channel_manager_iface_init)
  G_ADD_PRIVATE(FbAccountVerifyManager))

/* properties */
enum
{
  PROP_CONNECTION = 1,
  LAST_PROPERTY
};

static void
fb_account_verify_manager_foreach_channel (TpChannelManager *manager,
                                                 TpExportableChannelFunc foreach,
                                                 gpointer user_data)
{
  FbAccountVerifyManagerPrivate *priv = PRIVATE(manager);

  if (priv->channel &&
      !tp_base_channel_is_destroyed(TP_BASE_CHANNEL(priv->channel)))
  {
    foreach(TP_EXPORTABLE_CHANNEL(priv->channel), user_data);
  }
}

static void
channel_manager_iface_init(TpChannelManagerIface *iface)
{
  iface->foreach_channel = fb_account_verify_manager_foreach_channel;

  /* these channels are not requestable */
  iface->foreach_channel_class = NULL;
  iface->request_channel = NULL;
  iface->create_channel = NULL;
  iface->ensure_channel = NULL;
}

static void
fb_account_verify_manager_init(FbAccountVerifyManager *self)
{}

static void
fb_account_verify_manager_get_property(GObject *object, guint property_id,
                                             GValue *value, GParamSpec *pspec)
{
  FbAccountVerifyManagerPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_CONNECTION:
    {
      g_value_set_object(value, priv->conn);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

static void
fb_account_verify_manager_set_property(GObject *object,
                                             guint property_id,
                                             const GValue *value,
                                             GParamSpec *pspec)
{
  FbAccountVerifyManagerPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_CONNECTION:
    {
      priv->conn = g_value_get_object(value);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
    }
  }
}

static void
fb_account_verify_manager_close_all(FbAccountVerifyManager *self)
{
  FbAccountVerifyManagerPrivate *priv = PRIVATE(self);

  if (!priv->channel)
    return;

  FB_DEBUG("closing %p", priv->channel);

  tp_base_channel_close(TP_BASE_CHANNEL(priv->channel));
}

static void
conn_status_changed_cb(TpBaseConnection *conn, guint status, guint reason,
                       FbAccountVerifyManager *self)
{
  switch (status)
  {
    case TP_CONNECTION_STATUS_DISCONNECTED:
    {
      fb_account_verify_manager_close_all(self);
      break;
    }
  }
}

static void
fb_account_verify_manager_constructed(GObject *object)
{
  void (*constructed)(GObject *) =
    G_OBJECT_CLASS(fb_account_verify_manager_parent_class)->constructed;
  FbAccountVerifyManagerPrivate *priv = PRIVATE(object);

  if (constructed)
    constructed(object);

  tp_g_signal_connect_object(priv->conn, "status-changed",
                             G_CALLBACK(conn_status_changed_cb), object, 0);
}

static void
fb_account_verify_manager_dispose (GObject *object)
{
  FbAccountVerifyManager *self = FB_ACCOUNT_VERIFY_MANAGER(object);
  FbAccountVerifyManagerPrivate *priv = PRIVATE(self);

  if (priv->dispose_has_run)
    return;

  FB_DEBUG("dispose called");

  priv->dispose_has_run = TRUE;

  fb_account_verify_manager_close_all(self);

  if (G_OBJECT_CLASS(fb_account_verify_manager_parent_class)->dispose)
    G_OBJECT_CLASS(fb_account_verify_manager_parent_class)->dispose(object);
}

static void
fb_account_verify_manager_class_init(
  FbAccountVerifyManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = fb_account_verify_manager_constructed;
  object_class->dispose = fb_account_verify_manager_dispose;

  object_class->get_property = fb_account_verify_manager_get_property;
  object_class->set_property = fb_account_verify_manager_set_property;

  g_object_class_install_property(
    object_class, PROP_CONNECTION,
    g_param_spec_object("connection",
                        "TpBaseConnection object",
                        "The connection object that owns this channel manager",
                        TP_TYPE_BASE_CONNECTION,
                        G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE |
                        G_PARAM_STATIC_STRINGS));
}

FbAccountVerifyManager *
fb_account_verify_manager_new(TpBaseConnection *connection)
{
  g_return_val_if_fail(TP_IS_BASE_CONNECTION(connection), NULL);

  return g_object_new(FB_TYPE_ACCOUNT_VERIFY_MANAGER,
                      "connection", connection,
                      NULL);
}

static void
channel_closed_cb(GObject *chan, FbAccountVerifyManager *manager)
{
  FbAccountVerifyManagerPrivate *priv = PRIVATE(manager);

  tp_channel_manager_emit_channel_closed_for_object(
    manager, TP_EXPORTABLE_CHANNEL(chan));

  tp_clear_object(&priv->channel);
}

static void
channel_finished_cb(FbAccountVerifyChannel *channel, gboolean verified,
                    guint domain, gint code, const gchar *message,
                    gpointer user_data)
{
  GSimpleAsyncResult *result = user_data;

  if (domain > 0)
  {
    GError *error = g_error_new(domain, code, "%s", message);

    FB_DEBUG("Failed: %s", error->message);
    g_simple_async_result_set_from_error(result, error);
    g_error_free(error);
  }
  else
    g_simple_async_result_set_op_res_gboolean(result, verified);

  g_simple_async_result_complete(result);
  g_object_unref(result);
}

void
fb_account_verify_manager_verify_async(FbAccountVerifyManager *self,
                                             const gchar *verify_url,
                                             const gchar *title,
                                             const gchar *message,
                                             GAsyncReadyCallback callback,
                                             gpointer user_data)
{
  FbAccountVerifyManagerPrivate *priv = PRIVATE(self);
  gchar *object_path = g_strdup_printf(
    "%s/FbAccountVerifyChannel",
    tp_base_connection_get_object_path(priv->conn));
  FbAccountVerifyChannel *channel;
  GSimpleAsyncResult *result =
    g_simple_async_result_new(G_OBJECT(self),
                              callback,
                              user_data,
                              fb_account_verify_manager_verify_async);

  channel = g_object_new(FB_TYPE_ACCOUNT_VERIFY_CHANNEL,
                         "connection", priv->conn,
                         "object-path", object_path,
                         "handle", 0,
                         "requested", FALSE,
                         "initiator-handle",
                         tp_base_connection_get_self_handle(priv->conn),
                         "verify-url", verify_url,
                         "title", title,
                         "message", message,
                         NULL);

  priv->channel = g_object_ref(channel);

  tp_g_signal_connect_object(priv->channel, "closed",
                             G_CALLBACK(channel_closed_cb), self, 0);
  tp_g_signal_connect_object(priv->channel, "finished",
                             G_CALLBACK(channel_finished_cb),
                             g_object_ref(result), 0);

  tp_base_channel_register((TpBaseChannel *)priv->channel);

  tp_channel_manager_emit_new_channel(
    self, TP_EXPORTABLE_CHANNEL(priv->channel), NULL);

  g_free(object_path);
  g_object_unref(channel);
  g_object_unref(result);
}

gboolean
fb_account_verify_manager_verify_finish(
  FbAccountVerifyManager *self,
  GAsyncResult *result,
  GError **error)
{
  GSimpleAsyncResult *simple = (GSimpleAsyncResult *)result;

  if (g_simple_async_result_propagate_error(simple, error))
    return FALSE;

  g_return_val_if_fail(
    g_simple_async_result_is_valid(
      result,
      G_OBJECT(self),
      fb_account_verify_manager_verify_async),
    FALSE);

  return g_simple_async_result_get_op_res_gboolean(simple);
}
