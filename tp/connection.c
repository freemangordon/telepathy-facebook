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

#include "connection-data.h"
#include "connection.h"
#include "presence.h"
#include "avatars.h"

#include "debug.h"

#include "bitlbee.h"

#include "facebook-api.h"
#include "facebook-data.h"
#include "facebook-mqtt.h"
#include "facebook-util.h"

#include "account-verify-manager.h"
#include "contact-list.h"

struct _FbConnectionPrivate
{
  /** facebook user */
  gchar *fb_id;

  /** facebook password */
  gchar *password;

  FbAccountVerifyManager *account_verify;

  FbApi *api;
  GHashTable *data;

  FbContactList *contact_list;

  /** if fb_connection_dispose() has already run once */
  gboolean dispose_has_run;
};

typedef struct _FbConnectionPrivate FbConnectionPrivate;

G_DEFINE_TYPE_WITH_CODE(FbConnection,
                        fb_connection,
                        TP_TYPE_BASE_CONNECTION,
                        G_IMPLEMENT_INTERFACE(
                          TP_TYPE_SVC_CONNECTION_INTERFACE_PRESENCE,
                          tp_presence_mixin_iface_init);
                        G_IMPLEMENT_INTERFACE(
                          TP_TYPE_SVC_CONNECTION_INTERFACE_SIMPLE_PRESENCE,
                          tp_presence_mixin_simple_presence_iface_init);
                        G_IMPLEMENT_INTERFACE(
                          TP_TYPE_SVC_CONNECTION_INTERFACE_CONTACT_LIST,
                          tp_base_contact_list_mixin_list_iface_init);
                        G_IMPLEMENT_INTERFACE(
                          TP_TYPE_SVC_CONNECTION_INTERFACE_CONTACTS,
                          tp_contacts_mixin_iface_init);
                        G_IMPLEMENT_INTERFACE(
                          TP_TYPE_SVC_CONNECTION_INTERFACE_AVATARS,
                          fb_connection_avatars_iface_init);
                        G_ADD_PRIVATE(FbConnection);
)

enum
{
  PROP_FB_ID = 1,
  PROP_PASSWORD,
  LAST_PROPERTY_ENUM
};

#define PRIVATE(o) ((FbConnectionPrivate *) \
  (fb_connection_get_instance_private((FbConnection *)(o))));

static gchar *
_contact_normalize_func(TpHandleRepoIface *repo,
                        const gchar *id,
                        gpointer ctx,
                        GError **error)
{
  return g_strdup(id);
}

static gchar *
_chat_normalize_func(TpHandleRepoIface *repo,
                     const gchar *id,
                     gpointer ctx,
                     GError **error)
{
  return g_strdup(id);
}

static void
_iface_create_handle_repos(TpBaseConnection *self, TpHandleRepoIface **repos)
{
  for (int i = 0; i < NUM_TP_HANDLE_TYPES; i++)
    repos[i] = NULL;

  repos[TP_HANDLE_TYPE_CONTACT] =
    g_object_new(TP_TYPE_DYNAMIC_HANDLE_REPO,
                 "handle-type",
                 TP_HANDLE_TYPE_CONTACT,
                 "normalize-function",
                 _contact_normalize_func,
                 "default-normalize-context",
                 NULL,
                 NULL);

  repos[TP_HANDLE_TYPE_ROOM] =
    g_object_new(TP_TYPE_DYNAMIC_HANDLE_REPO,
                 "handle-type",
                 TP_HANDLE_TYPE_ROOM,
                 "normalize-function",
                 _chat_normalize_func,
                 "default-normalize-context",
                 NULL,
                 NULL);
}

static gchar *
_iface_get_unique_connection_name(TpBaseConnection *base)
{
  FbConnection *self = FB_CONNECTION(base);
  FbConnectionPrivate *priv = PRIVATE(self);

  return g_strdup_printf("%s@facebook%p", priv->fb_id, self);
}

static GPtrArray *
_iface_create_channel_managers(TpBaseConnection *base)
{
  FbConnectionPrivate *priv = PRIVATE(base);
  GPtrArray *managers = g_ptr_array_sized_new(2);

  priv->account_verify = fb_account_verify_manager_new(base);
  g_ptr_array_add(managers, priv->account_verify);

  priv->contact_list = fb_contact_list_new(base);
  g_ptr_array_add(managers, priv->contact_list);

  return managers;
}

static const gchar *interfaces_always_present[] =
{
  TP_IFACE_CONNECTION_INTERFACE_CONTACT_LIST,
  TP_IFACE_CONNECTION_INTERFACE_AVATARS,
  TP_IFACE_CONNECTION_INTERFACE_CONTACTS,
  TP_IFACE_CONNECTION_INTERFACE_REQUESTS,

  NULL
};

const gchar *const *
fb_connection_get_implemented_interfaces(void)
{
  /* we don't have any conditionally-implemented interfaces yet */
  return interfaces_always_present;
}

static GPtrArray *
get_interfaces_always_present (TpBaseConnection *base)
{
  GPtrArray *interfaces;
  const gchar **iter;

  interfaces = TP_BASE_CONNECTION_CLASS(
    fb_connection_parent_class)->get_interfaces_always_present(base);

  for (iter = interfaces_always_present; *iter != NULL; iter++)
    g_ptr_array_add(interfaces, (gchar *)*iter);

  return interfaces;
}

static void
fb_connection_set_property(GObject *obj, guint prop_id,
                           const GValue *value, GParamSpec *pspec)
{
  FbConnection *self = FB_CONNECTION(obj);
  FbConnectionPrivate *priv = PRIVATE(self);

  switch (prop_id)
  {
    case PROP_FB_ID:
    {
      g_free(priv->fb_id);
      priv->fb_id = g_value_dup_string(value);
      break;
    }
    case PROP_PASSWORD:
    {
      g_free(priv->password);
      priv->password = g_value_dup_string(value);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
      break;
    }
  }
}

static void
fb_connection_get_property(GObject *obj, guint prop_id, GValue *value,
                           GParamSpec *pspec) {
  FbConnection *self = FB_CONNECTION(obj);
  FbConnectionPrivate *priv = PRIVATE(self);

  switch (prop_id)
  {
    case PROP_FB_ID:
    {
      g_value_set_string(value, priv->fb_id);
      break;
    }
    case PROP_PASSWORD:
    {
      g_value_set_string(value, priv->password);
      break;
    }
    default:
    {
      G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, prop_id, pspec);
      break;
    }
  }
}

static void
fb_connection_dispose(GObject *object)
{
  FbConnection *self = FB_CONNECTION(object);
  FbConnectionPrivate *priv = PRIVATE(self);

  if (priv->dispose_has_run)
    return;

  priv->dispose_has_run = TRUE;

  if (G_OBJECT_CLASS(fb_connection_parent_class)->dispose)
    G_OBJECT_CLASS(fb_connection_parent_class)->dispose(object);
}

static void
fb_connection_finalize(GObject *object)
{
  FbConnection *self = FB_CONNECTION(object);
  FbConnectionPrivate *priv = PRIVATE(self);

  tp_clear_pointer(&priv->fb_id, g_free);
  tp_clear_pointer(&priv->password, g_free);

  tp_contacts_mixin_finalize(object);
  tp_presence_mixin_finalize(object);

  G_OBJECT_CLASS(fb_connection_parent_class)->finalize(object);
}

static void
fb_connection_aliasing_fill_contact_attributes(GObject *object,
                                               const GArray *contacts,
                                               GHashTable *attributes_hash)
{
  FbConnection *self = FB_CONNECTION(object);
  FbConnectionPrivate *priv = PRIVATE(self);

  for (guint i = 0; i < contacts->len; i++)
  {
    TpHandle handle = g_array_index(contacts, TpHandle, i);
    gchar uid[FB_ID_STRMAX];
    const FbContact *c = fb_contact_list_get_user(priv->contact_list, handle);
    const gchar *alias = c->name;

    if (!alias)
    {
      FB_ID_TO_STR(c->uid, uid);
      alias = uid;
    }

    tp_contacts_mixin_set_contact_attribute(
      attributes_hash, handle, TP_TOKEN_CONNECTION_INTERFACE_ALIASING_ALIAS,
      tp_g_value_slice_new_string(alias));
  }
}

static void
fb_connection_constructed(GObject *object)
{
  TpBaseConnection *base = TP_BASE_CONNECTION(object);

  void (*constructed)(GObject *) =
    G_OBJECT_CLASS(fb_connection_parent_class)->constructed;

  if (constructed != NULL)
    constructed(object);

  tp_contacts_mixin_init(object,
                         G_STRUCT_OFFSET(FbConnection, contacts));

  tp_base_connection_register_with_contacts_mixin(base);
  tp_base_contact_list_mixin_register_with_contacts_mixin(base);

  tp_contacts_mixin_add_contact_attributes_iface(
        object, TP_IFACE_CONNECTION_INTERFACE_ALIASING,
        fb_connection_aliasing_fill_contact_attributes);

  fb_connection_avatars_init(object);
  fb_connection_presence_init(object);
}

static void
fb_connection_save_data(FbConnectionPrivate *priv)
{
  static const gchar *props[] =
  {
    "cid",
    "did",
    "stoken",
    "token",
    "machine_id",
    "login_first_factor",
    "twofactor_code"
  };

  guint64 uid, mid;

  for (int i = 0; i < G_N_ELEMENTS(props); i++)
  {
    gchar *val;

    g_object_get(priv->api, props[i], &val, NULL);

    if (val)
      fb_connection_data_set_string(priv->data, (gchar *)props[i], val);
  }

  g_object_get(G_OBJECT(priv->api), "mid", &mid, "uid", &uid, NULL);

  fb_connection_data_set_uint64(priv->data, "mid", mid);
  fb_connection_data_set_uint64(priv->data, "uid", uid);

  fb_connection_data_save(priv->fb_id, priv->data);
}

static void
fb_api_auth_cb(FbApi *api, gpointer data)
{
  FbConnection *conn = FB_CONNECTION(data);
  FbConnectionPrivate *priv = PRIVATE(conn);

  fb_connection_save_data(priv);

  tp_base_contact_list_set_list_pending(
    TP_BASE_CONTACT_LIST(priv->contact_list));

  fb_api_contacts(priv->api);
}

static void
_account_verified_cb(GObject *source, GAsyncResult *result, gpointer user_data)
{
  FbConnection *conn = user_data;
  FbConnectionPrivate *priv = PRIVATE(conn);
  TpBaseConnection *base_conn = TP_BASE_CONNECTION(conn);
  gboolean account_verified;
  GError *error = NULL;

  account_verified = fb_account_verify_manager_verify_finish(
    FB_ACCOUNT_VERIFY_MANAGER(source),
    result, &error);

  if (error)
  {
    FB_DEBUG("account verify manager failed: %s", error->message);

    g_error_free(error);
    goto auth_failed;
  }

  if (account_verified)
  {
    fb_api_auth(priv->api, priv->fb_id, priv->password, NULL);
    return;
  }

auth_failed:

  if (tp_base_connection_get_status(base_conn) !=
      TP_CONNECTION_STATUS_DISCONNECTED)
  {
    tp_base_connection_change_status(
      base_conn, TP_CONNECTION_STATUS_DISCONNECTED,
      TP_CONNECTION_STATUS_REASON_AUTHENTICATION_FAILED);
  }
}

static void
fb_api_account_verify_cb(FbApi *api , const gchar *url, const gchar *title,
                         const gchar *message, gpointer user_data)
{
  FbConnection *conn = FB_CONNECTION(user_data);
  FbConnectionPrivate *priv = PRIVATE(conn);

  FB_DEBUG("%s", title);

  fb_account_verify_manager_verify_async(priv->account_verify, url, title,
                                         message, _account_verified_cb, conn);
}

static void
fb_api_connect_cb(FbApi *api, gpointer user_data)
{
  FbConnection *conn = FB_CONNECTION(user_data);
  FbConnectionPrivate *priv = PRIVATE(conn);
  TpBaseConnection *base_conn = TP_BASE_CONNECTION(conn);

  fb_connection_save_data(priv);

  tp_base_connection_change_status(base_conn, TP_CONNECTION_STATUS_CONNECTED,
                                   TP_CONNECTION_STATUS_REASON_REQUESTED);
  tp_base_contact_list_set_list_received(
    TP_BASE_CONTACT_LIST(priv->contact_list));

//  if (set_getbool(&acct->set, "show_unread"))
//    fb_api_unread(api);
}

static void
fb_cb_api_error(FbApi *api, GError *error, gpointer user_data)
{
  FbConnectionPrivate *priv = PRIVATE(user_data);
  TpConnectionStatusReason reason = TP_CONNECTION_STATUS_REASON_NETWORK_ERROR;
  TpBaseContactList *contact_list = TP_BASE_CONTACT_LIST(priv->contact_list);

  FB_DEBUG("%s", error->message);

  if (error->domain == FB_API_ERROR)
  {
    switch (error->code)
    {
      case FB_API_ERROR_AUTH:
      {
        reason = TP_CONNECTION_STATUS_REASON_AUTHENTICATION_FAILED;
        break;
      }
      default:
      {
        reason = TP_CONNECTION_STATUS_REASON_NONE_SPECIFIED;
        break;
      }
    }
  }

  if (tp_base_contact_list_get_state(contact_list, NULL) ==
      TP_CONTACT_LIST_STATE_WAITING)
  {
    tp_base_contact_list_set_list_failed(contact_list, error->domain,
                                         error->code, error->message);
  }

  tp_base_connection_change_status(TP_BASE_CONNECTION(user_data),
                                   TP_CONNECTION_STATUS_DISCONNECTED, reason);
}

static void
fb_cb_api_contacts(FbApi *api, GSList *users, gboolean complete,
                   gpointer user_data)
{
  TpBaseConnection *base_conn = TP_BASE_CONNECTION(user_data);
  FbConnection *conn = FB_CONNECTION(user_data);
  FbConnectionPrivate *priv = PRIVATE(conn);

  fb_contact_list_fb_contacts_changed(priv->contact_list, api, users);

  if (!complete)
    return;

  if (tp_base_connection_get_status(base_conn) !=
      TP_CONNECTION_STATUS_CONNECTED)
  {
    fb_api_connect(api, FALSE);
  }

//    fb_sync_contacts_add_timeout(fata);
}

static void
fb_cb_api_presences(FbApi *api, GSList *presences, gpointer user_data)
{
  fb_contact_list_fb_presences_changed(user_data, presences);
}

static gboolean
_iface_start_connecting(TpBaseConnection *self, GError **error)
{
  FbConnection *conn = FB_CONNECTION(self);
  FbConnectionPrivate *priv = PRIVATE(conn);
  TpHandleRepoIface *contact_repo =
      tp_base_connection_get_handles(self, TP_HANDLE_TYPE_CONTACT);
  TpHandle self_handle;

  g_assert(priv->fb_id != NULL);

  if (priv->api)
  {
    FB_DEBUG("api already created!");
    g_set_error(error, TP_ERROR, TP_ERROR_NOT_AVAILABLE,
                "connection already open!");
    return FALSE;
  }

  priv->data = fb_connection_data_load(priv->fb_id);
  priv->api = fb_api_new();

  g_object_set(
    priv->api,
    "cid",
    fb_connection_data_get_string(priv->data, "cid"),
    "did",
    fb_connection_data_get_string(priv->data, "did"),
    "stoken",
    fb_connection_data_get_string(priv->data, "stoken"),
    "token",
    fb_connection_data_get_string(priv->data, "token"),
    "mid",
    fb_connection_data_get_uint64(priv->data, "mid", 0),
    "uid",
    fb_connection_data_get_int64(priv->data, "uid", 0),
    "work",
    fb_connection_data_get_bool(priv->data, "work", 0),
    "machine_id",
    fb_connection_data_get_string(priv->data, "machine_id"),
    "login_first_factor",
    fb_connection_data_get_string(priv->data, "login_first_factor"),
    "twofactor_code",
    fb_connection_data_get_string(priv->data, "twofactor_code"),
    NULL);

  fb_api_rehash(priv->api);

  g_signal_connect(priv->api, "auth",
                   G_CALLBACK(fb_api_auth_cb), self);
  g_signal_connect(priv->api, "account-verify",
                   G_CALLBACK(fb_api_account_verify_cb), self);
  g_signal_connect(priv->api, "connect",
                   G_CALLBACK(fb_api_connect_cb), self);
  g_signal_connect(priv->api, "error",
                   G_CALLBACK(fb_cb_api_error), self);
  g_signal_connect(priv->api, "contacts",
                   G_CALLBACK(fb_cb_api_contacts), self);
  g_signal_connect(priv->api, "presences",
                   G_CALLBACK(fb_cb_api_presences), self);

/*  g_signal_connect(priv->api, "contact",
                   G_CALLBACK(fb_cb_api_contact), self);*/

  self_handle = tp_handle_ensure(contact_repo, priv->fb_id, NULL, error);

  if (self_handle == 0)
    return FALSE;

  tp_base_connection_set_self_handle(self, self_handle);
  tp_base_connection_change_status(self,
                                   TP_CONNECTION_STATUS_CONNECTING,
                                   TP_CONNECTION_STATUS_REASON_REQUESTED);

  fb_api_auth(priv->api, priv->fb_id, priv->password, NULL);

  return TRUE;
}

static void
_iface_shut_down(TpBaseConnection *base)
{
  FbConnection *self = FB_CONNECTION(base);
  FbConnectionPrivate *priv = PRIVATE(self);

  fb_api_disconnect(priv->api);

  tp_clear_object(&priv->api);
  tp_clear_pointer(&priv->data, g_hash_table_destroy);

  tp_base_connection_finish_shutdown(base);
}

static void
fb_connection_class_init(FbConnectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  TpBaseConnectionClass *parent_class = TP_BASE_CONNECTION_CLASS(klass);

  object_class->constructed = fb_connection_constructed;
  object_class->set_property = fb_connection_set_property;
  object_class->get_property = fb_connection_get_property;
  object_class->dispose = fb_connection_dispose;
  object_class->finalize = fb_connection_finalize;

  parent_class->create_handle_repos = _iface_create_handle_repos;
  parent_class->get_unique_connection_name = _iface_get_unique_connection_name;
  parent_class->create_channel_factories = NULL;
  parent_class->create_channel_managers = _iface_create_channel_managers;
  parent_class->connecting = NULL;
  parent_class->connected = NULL;
  //parent_class->disconnected = _iface_disconnected;
  parent_class->shut_down = _iface_shut_down;
  parent_class->start_connecting = _iface_start_connecting;
  parent_class->get_interfaces_always_present = get_interfaces_always_present;

  g_object_class_install_property(
    object_class, PROP_FB_ID,
    g_param_spec_string("facebook-id",
                        "Fb user",
                        "Email or telephone number of the facebook user",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
    object_class, PROP_PASSWORD,
    g_param_spec_string("password",
                        "Fb password",
                        "Password to authenticate to facebook with",
                        NULL,
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  tp_contacts_mixin_class_init(
    object_class, G_STRUCT_OFFSET(FbConnectionClass, contacts_class));
  tp_base_contact_list_mixin_class_init(parent_class);
  fb_connection_presence_class_init(object_class);
}

static void
fb_connection_init(FbConnection *self)
{}

FbContactList *
fb_connection_get_contact_list(FbConnection *self)
{
  FbConnectionPrivate *priv = PRIVATE(self);

  return priv->contact_list;
}
