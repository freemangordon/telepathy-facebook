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
#ifndef FACEBOOKACCOUNTVERIFYUI_H
#define FACEBOOKACCOUNTVERIFYUI_H

#include <QWebEngineProfile>
#include <QWebEngineView>

class FacebookAccountVerifyUI : public QWebEngineView
{
  Q_OBJECT

public:
  explicit FacebookAccountVerifyUI(const QString &url);

protected:
  void closeEvent(QCloseEvent *event);

Q_SIGNALS:
  void verified(bool ok);
};

#endif // FACEBOOKACCOUNTVERIFYUI_H
