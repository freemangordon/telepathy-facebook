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

#include "connection.h"
#include "protocol.h"

#include <dbus/dbus-glib.h>
#include <dbus/dbus-protocol.h>

#define PROTOCOL_NAME "facebook"
#define ICON_NAME "im-" PROTOCOL_NAME
#define ENGLISH_NAME "Fb"
#define VCARD_FIELD_NAME "x-" PROTOCOL_NAME

G_DEFINE_TYPE(FbProtocol, fb_protocol, TP_TYPE_BASE_PROTOCOL)

static gboolean
filter_fb_id(const TpCMParamSpec *paramspec G_GNUC_UNUSED,
                   GValue *value, GError **error)
{
  const gchar *input = g_value_get_string(value);

  g_autoptr(GRegex) regex_email = NULL;
  g_autoptr(GRegex) regex_phone = NULL;

  g_assert(value);
  g_assert(G_VALUE_HOLDS_STRING(value));

  // Regex for email
  regex_email = g_regex_new("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$",
                            0, 0, NULL);

  if (g_regex_match(regex_email, input, 0, NULL))
    return TRUE;

  // Regex for phone number
  regex_phone = g_regex_new("^\\+?[0-9]{7,15}$", 0, 0, NULL);

  if (g_regex_match(regex_phone, input, 0, NULL))
    return TRUE;

  // Neither matched
  g_set_error(error,
              TP_ERROR,
              TP_ERROR_INVALID_HANDLE,
              "Invalid Fb login: must be an email address or phone number (digits only). Got: '%s'",
              input);
  return FALSE;
}

static const TpCMParamSpec fb_params[] =
{
  { "account", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING,
    TP_CONN_MGR_PARAM_FLAG_REQUIRED, NULL,
    0 /* unused */, filter_fb_id, NULL },
  { "password", DBUS_TYPE_STRING_AS_STRING, G_TYPE_STRING,
    TP_CONN_MGR_PARAM_FLAG_SECRET | TP_CONN_MGR_PARAM_FLAG_REQUIRED,
    NULL, 0, NULL, NULL },
  { NULL, NULL, 0, 0, NULL, 0 }
};

static void
fb_protocol_init(FbProtocol *self G_GNUC_UNUSED)
{}

static const TpCMParamSpec *
get_parameters(TpBaseProtocol *self G_GNUC_UNUSED)
{
  return fb_params;
}

static TpBaseConnection *
new_connection(TpBaseProtocol *protocol G_GNUC_UNUSED, GHashTable *params,
               GError **error G_GNUC_UNUSED)
{
  return g_object_new(FB_TYPE_CONNECTION,
                      "protocol", PROTOCOL_NAME,
                      "facebook-id", tp_asv_get_string(params, "account"),
                      "password", tp_asv_get_string(params, "password"),
                      NULL);
}

static GPtrArray *
get_interfaces_array(TpBaseProtocol *self)
{
  GPtrArray *interfaces;

  interfaces = TP_BASE_PROTOCOL_CLASS(
    fb_protocol_parent_class)->get_interfaces_array(self);

  return interfaces;
}

static void
get_connection_details(TpBaseProtocol *self,
                       GStrv *connection_interfaces,
                       GType **channel_managers,
                       gchar **icon_name,
                       gchar **english_name,
                       gchar **vcard_field)
{
  if (connection_interfaces != NULL)
  {
    *connection_interfaces = g_strdupv(
      (GStrv)fb_connection_get_implemented_interfaces());
  }

  if (channel_managers != NULL)
  {
    GType types[] =
    {
//      FB_TYPE_IM_MANAGER,
//      FB_TYPE_MUC_MANAGER,
      G_TYPE_INVALID
    };

    *channel_managers = g_memdup(types, sizeof(types));
  }

  if (icon_name)
    *icon_name = g_strdup(ICON_NAME);

  if (vcard_field)
    *vcard_field = g_strdup(VCARD_FIELD_NAME);

  if (english_name)
    *english_name = g_strdup(ENGLISH_NAME);
}

static GStrv
dup_authentication_types(TpBaseProtocol *base G_GNUC_UNUSED)
{
  const gchar *const types[] =
  {
    NULL,
  };

  return g_strdupv((GStrv)types);
}

static void
fb_protocol_class_init(FbProtocolClass *klass)
{
  TpBaseProtocolClass *base_class = (TpBaseProtocolClass *)klass;

  base_class->get_parameters = get_parameters;
  base_class->new_connection = new_connection;
  base_class->get_interfaces_array = get_interfaces_array;
  base_class->get_connection_details = get_connection_details;
  base_class->dup_authentication_types = dup_authentication_types;
}

TpBaseProtocol *
fb_protocol_new(void)
{
  return g_object_new(FB_TYPE_PROTOCOL,
                      "name", PROTOCOL_NAME,
                      NULL);
}
