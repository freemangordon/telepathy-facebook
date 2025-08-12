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
#include "svc.h"
#include "debug.h"

struct _FbAccountVerifyChannelClass
{
  TpBaseChannelClass parent_class;

  TpDBusPropertiesMixinClass properties_class;
};

struct _FbAccountVerifyChannel
{
  TpBaseChannel parent;
};

struct _FbAccountVerifyChannelPrivate
{
  gchar *verify_url;
  gchar *message;
  gchar *title;
};

typedef struct _FbAccountVerifyChannelPrivate
  FbAccountVerifyChannelPrivate;

static void
fb_acct_vrfy_iface_init(gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE(
  FbAccountVerifyChannel,
  fb_account_verify_channel,
  TP_TYPE_BASE_CHANNEL,
  G_IMPLEMENT_INTERFACE(FB_TYPE_SVC_CHANNEL_INTERFACE_ACCOUNT_VERIFY,
                        fb_acct_vrfy_iface_init)
  G_IMPLEMENT_INTERFACE(TP_TYPE_SVC_CHANNEL_TYPE_SERVER_AUTHENTICATION, NULL)
  G_ADD_PRIVATE(FbAccountVerifyChannel)
)

#define PRIVATE(o) \
  ((FbAccountVerifyChannelPrivate *) \
   fb_account_verify_channel_get_instance_private( \
     FB_ACCOUNT_VERIFY_CHANNEL(o)))

/* properties */
enum
{
  PROP_AUTHENTICATION_METHOD = 1,
  PROP_VERIFY_URL,
  PROP_TITLE,
  PROP_MESSAGE,

  LAST_PROPERTY,
};

/* signals */
enum
{
  FINISHED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

#define FB_ACCOUNT_VERIFY_IFACE \
  "org.freedesktop.Telepathy.Channel.Interface.Facebook.AccountVerify"

static void
fb_account_verify_channel_init(FbAccountVerifyChannel *self)
{}

static void
fb_account_verify_channel_set_property(GObject *object,
                                       guint property_id,
                                       const GValue *value,
                                       GParamSpec *pspec)
{
  FbAccountVerifyChannelPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_VERIFY_URL:
    {
      g_free(priv->verify_url);
      priv->verify_url = g_strdup(g_value_get_string(value));
      break;
    }
    case PROP_MESSAGE:
    {
      g_free(priv->message);
      priv->message = g_strdup(g_value_get_string(value));
      break;
    }
    case PROP_TITLE:
    {
      g_free(priv->title);
      priv->title = g_strdup(g_value_get_string(value));
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
fb_account_verify_channel_get_property(GObject *object,
                                       guint property_id,
                                       GValue *value,
                                       GParamSpec *pspec)
{
  FbAccountVerifyChannelPrivate *priv = PRIVATE(object);

  switch (property_id)
  {
    case PROP_AUTHENTICATION_METHOD:
    {
      g_value_set_static_string(value, FB_ACCOUNT_VERIFY_IFACE);
      break;
    }

    case PROP_VERIFY_URL:
    {
      g_value_set_string(value, priv->verify_url);
      break;
    }
    case PROP_MESSAGE:
    {
      g_value_set_string(value, priv->message);
      break;
    }
    case PROP_TITLE:
    {
      g_value_set_string(value, priv->title);
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
fb_account_verify_channel_finalize(GObject *object)
{
  FbAccountVerifyChannelPrivate *priv = PRIVATE(object);

  tp_clear_pointer(&priv->verify_url, g_free);
  tp_clear_pointer(&priv->message, g_free);
  tp_clear_pointer(&priv->title, g_free);

  if (G_OBJECT_CLASS(fb_account_verify_channel_parent_class)->finalize)
    G_OBJECT_CLASS(fb_account_verify_channel_parent_class)->finalize(
      object);
}

static void
fb_account_verify_channel_close(TpBaseChannel *base)
{
  if (tp_base_channel_is_destroyed(base))
    return;

  g_signal_emit(base, signals[FINISHED], 0, NULL, TP_ERROR,
                TP_ERROR_AUTHENTICATION_FAILED,
                "FbAccountVerifyChannel channel was closed");

  FB_DEBUG("Closing channel");
  tp_base_channel_destroyed(base);
}

static void
fb_account_verify_channel_fill_immutable_properties(TpBaseChannel *chan,
                                                    GHashTable *properties)
{
  TpBaseChannelClass *klass = TP_BASE_CHANNEL_CLASS(
    fb_account_verify_channel_parent_class);

  klass->fill_immutable_properties(chan, properties);

  tp_dbus_properties_mixin_fill_properties_hash(
    G_OBJECT(chan), properties,
    TP_IFACE_CHANNEL_TYPE_SERVER_AUTHENTICATION, "AuthenticationMethod",
    FB_ACCOUNT_VERIFY_IFACE, "VerifyUrl",
    FB_ACCOUNT_VERIFY_IFACE, "Title",
    FB_ACCOUNT_VERIFY_IFACE, "Message",
    NULL);
}

static GPtrArray *
fb_account_verify_channel_get_interfaces (TpBaseChannel *base)
{
  GPtrArray *interfaces;

  interfaces = TP_BASE_CHANNEL_CLASS(
    fb_account_verify_channel_parent_class)->get_interfaces(base);

  g_ptr_array_add(interfaces, FB_ACCOUNT_VERIFY_IFACE);

  return interfaces;
}

static void
fb_account_verify_channel_class_init(FbAccountVerifyChannelClass *klass)
{
  TpBaseChannelClass *chan_class = TP_BASE_CHANNEL_CLASS(klass);
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->set_property = fb_account_verify_channel_set_property;
  object_class->get_property = fb_account_verify_channel_get_property;
  object_class->finalize = fb_account_verify_channel_finalize;

  static TpDBusPropertiesMixinPropImpl server_fb_account_verify_props[] =
  {
    { "AuthenticationMethod", "authentication-method", NULL },
    { NULL }
  };

  static TpDBusPropertiesMixinPropImpl
    fb_account_verify_channel_properties[] =
  {
    { "VerifyUrl", "verify-url", NULL },
    { "Title", "title", NULL },
    { "Message", "message", NULL },
    { NULL }
  };

  static TpDBusPropertiesMixinIfaceImpl prop_interfaces[] =
  {
    {
      TP_IFACE_CHANNEL_TYPE_SERVER_AUTHENTICATION,
      tp_dbus_properties_mixin_getter_gobject_properties,
      NULL,
      server_fb_account_verify_props,
    },
    {
      FB_ACCOUNT_VERIFY_IFACE,
      tp_dbus_properties_mixin_getter_gobject_properties,
      NULL,
      fb_account_verify_channel_properties,
    },
    { NULL }
  };

  chan_class->channel_type = TP_IFACE_CHANNEL_TYPE_SERVER_AUTHENTICATION;
  chan_class->target_handle_type = TP_HANDLE_TYPE_NONE;
  chan_class->get_interfaces = fb_account_verify_channel_get_interfaces;
  chan_class->close = fb_account_verify_channel_close;
  chan_class->fill_immutable_properties =
    fb_account_verify_channel_fill_immutable_properties;

  g_object_class_install_property(
    object_class, PROP_AUTHENTICATION_METHOD,
    g_param_spec_string("authentication-method",
                        "Authentication method",
                        "Interface of authentication method",
                        "",
                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
    object_class, PROP_VERIFY_URL,
    g_param_spec_string("verify-url",
                        "Account verify url",
                        "Url to open for account verification",
                        "",
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
    object_class, PROP_TITLE,
    g_param_spec_string("title",
                        "Message title",
                        "Message title to display to the user",
                        "",
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property(
    object_class, PROP_MESSAGE,
    g_param_spec_string("message",
                        "Message",
                        "Message to display to the user",
                        "",
                        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  signals[FINISHED] = g_signal_new("finished",
                                   G_TYPE_FROM_CLASS(object_class),
                                   G_SIGNAL_RUN_LAST,
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   G_TYPE_NONE,
                                   4,
                                   G_TYPE_BOOLEAN,
                                   G_TYPE_UINT,
                                   G_TYPE_INT,
                                   G_TYPE_STRING);

  klass->properties_class.interfaces = prop_interfaces;

  tp_dbus_properties_mixin_class_init(
    object_class,
    G_STRUCT_OFFSET(TpBasePasswordChannelClass, properties_class));
}

FbAccountVerifyChannel *
fb_account_verify_channel_new(TpBaseConnection *connection,
                              const char *verify_url,
                              const char *title,
                              const char *message)
{
  return g_object_new(FB_TYPE_ACCOUNT_VERIFY_CHANNEL,
                      "connection", connection,
                      "verify-url", verify_url,
                      "title", title,
                      "message", message,
                      "target-handle-type", TP_HANDLE_TYPE_NONE,
                      NULL);
}

static void
fb_account_verify_channel_account_verified(
  FbSvcChannelInterfaceaccountverify *self,
  gboolean in_verified,
  DBusGMethodInvocation *context)
{
  FbAccountVerifyChannel *channel = FB_ACCOUNT_VERIFY_CHANNEL(self);

  g_signal_emit(channel, signals[FINISHED], 0, in_verified, 0, 0, NULL);

  fb_svc_channel_interface_account_verify_return_from_account_verified(
    context);
}

static void
fb_acct_vrfy_iface_init(gpointer g_iface, gpointer iface_data)
{
#define IMPLEMENT(x) \
  fb_svc_channel_interface_account_verify_implement_ ## x( \
    g_iface, fb_account_verify_channel_ ## x)
  IMPLEMENT(account_verified);
#undef IMPLEMENT
}
