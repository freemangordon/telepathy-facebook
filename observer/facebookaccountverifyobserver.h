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
#ifndef FACEBOOKACCOUNTVERIFYOBSERVER_H
#define FACEBOOKACCOUNTVERIFYOBSERVER_H

#include <QMap>
#include <QObject>

#include <telepathy-glib/telepathy-glib.h>

class FacebookAccountVerifyObserver : public QObject
{
  Q_OBJECT
public:
  explicit
  FacebookAccountVerifyObserver(QObject *parent = nullptr);
  ~FacebookAccountVerifyObserver();
  bool registerObserver();
  void verified(uintptr_t tag, bool ok);

Q_SIGNALS:
  void accountVerify(uintptr_t tag, const QString &url,
                     const QString &message);

protected:
  TpBaseClient *m_handler;

private:
  static void handle_channels_cb(TpSimpleHandler *handler,
                                 TpAccount *account,
                                 TpConnection *connection,
                                 GList *channels,
                                 GList *requests_satisfied,
                                 gint64 user_action_time,
                                 TpHandleChannelsContext *context,
                                 gpointer user_data);
};

#endif // FACEBOOKACCOUNTVERIFYOBSERVER_H
