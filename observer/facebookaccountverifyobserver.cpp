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
#include "facebookaccountverifyobserver.h"
#include <facebook-cli.h>

#include <QDebug>
#include <QEventLoop>

#define FB_ACCT_VRFY_IFACE \
  "org.freedesktop.Telepathy.Channel.Interface.FacebookAccountVerify"

void
FacebookAccountVerifyObserver::handle_channels_cb(
    TpSimpleHandler *handler,
    TpAccount *account,
    TpConnection *connection,
    GList *channels,
    GList *requests_satisfied,
    gint64 user_action_time,
    TpHandleChannelsContext *context,
    gpointer user_data)
{
  Q_UNUSED(handler);
  Q_UNUSED(account);
  Q_UNUSED(connection);
  Q_UNUSED(requests_satisfied);
  Q_UNUSED(user_action_time);

  FacebookAccountVerifyObserver *_this =
    (FacebookAccountVerifyObserver *)user_data;

  for (GList *l = channels; l; l = l->next)
  {
    TpChannel *channel = (TpChannel *)l->data;

    if (tp_proxy_has_interface(channel, FB_ACCT_VRFY_IFACE))
    {
      GVariant *props = tp_channel_dup_immutable_properties(channel);
      const gchar *url = NULL;

      g_variant_lookup(props, FB_ACCT_VRFY_IFACE ".VerifyUrl", "&s", &url);

      if (url)
      {
        const gchar *message = NULL;

        g_variant_lookup(props, FB_ACCT_VRFY_IFACE ".Message", "&s", &message);
        g_object_ref(channel);
        Q_EMIT(_this->accountVerify((uintptr_t)channel,
                                    QString::fromUtf8(url),
                                    QString::fromUtf8(message)));
      }
      else
        qWarning() << "VerifyUrl property is missing";

      g_variant_unref(props);
    }
  }

  tp_handle_channels_context_accept(context);
}

FacebookAccountVerifyObserver::FacebookAccountVerifyObserver(QObject *parent) :
  QObject(parent)
{
  tp_debug_set_flags(g_getenv("TP_DEBUG"));

  TpDBusDaemon *dbus = tp_dbus_daemon_dup(NULL);
  TpAutomaticClientFactory *factory = tp_automatic_client_factory_new(dbus);

  m_handler = tp_simple_handler_new_with_factory(
        TP_SIMPLE_CLIENT_FACTORY(factory),
        FALSE,
        TRUE,
        "Facebook.AccountVerify",
        FALSE,
        handle_channels_cb,
        this,
        NULL);

  g_object_unref(factory);
  g_object_unref(dbus);

  tp_base_client_take_handler_filter(
    m_handler,
    tp_asv_new(
      TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING,
        TP_IFACE_CHANNEL_TYPE_SERVER_AUTHENTICATION,
      TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT,
        TP_HANDLE_TYPE_NONE,
      NULL)
  );
}

FacebookAccountVerifyObserver::~FacebookAccountVerifyObserver()
{
  g_object_unref(m_handler);
}

bool
FacebookAccountVerifyObserver::registerObserver()
{
  GError *error = NULL;

  if (!tp_base_client_register(m_handler, &error))
  {
    qWarning("Failed to register Observer: %s\n", error->message);
    g_error_free(error);
    return false;
  }

  return true;
}

void
FacebookAccountVerifyObserver::verified(uintptr_t tag, bool ok)
{
  TpChannel *channel = (TpChannel *)tag;

  if (!tp_proxy_get_invalidated(channel))
  {
    facebook_cli_account_verified(channel, -1, ok, NULL, NULL, NULL, NULL);
    tp_cli_channel_call_close(channel, -1, NULL, NULL, NULL, NULL);
  }

  g_object_unref(channel);
}
