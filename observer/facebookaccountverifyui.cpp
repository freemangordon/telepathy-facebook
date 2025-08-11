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
#include "facebookaccountverifyui.h"

#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineHttpRequest>

#define REDIRECT_PREFIX "https://m.facebook.com/?deoia=1"

class AuthWebPage : public QWebEnginePage
{
Q_OBJECT

public:
  explicit AuthWebPage(QWebEngineProfile *profile,
                       QObject *parent = Q_NULLPTR) :
    QWebEnginePage(profile, parent)
  {
  }

protected:

  bool acceptNavigationRequest(const QUrl&url, NavigationType type,
                               bool isMainFrame) override
  {
    if (url.toString().startsWith(REDIRECT_PREFIX))
    {
      Q_EMIT(verified());
      return false; // Block further loading
    }

    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
  }

Q_SIGNALS:
  void verified();
};

FacebookAccountVerifyUI::FacebookAccountVerifyUI(const QString& url)
{
  QWebEngineProfile *privateProfile = new QWebEngineProfile(QString(), this);

  privateProfile->setHttpUserAgent(
    "Mozilla/5.0 (Linux; Android 16 Beta 2; Z832 Build/MMB29M) "
    "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/134.0.6998.35/36 "
    "Mobile Safari/537.36");

  AuthWebPage *page = new AuthWebPage(privateProfile, this);

  privateProfile->setParent(page);

  connect(page, &AuthWebPage::verified, [this]()
  {
    hide();
    Q_EMIT(verified(true));
  });

  setPage(page);

  QUrl _url(url);
  QWebEngineHttpRequest req(_url);

  load(req);
  show();
}

void
FacebookAccountVerifyUI::closeEvent(QCloseEvent *event)
{
  Q_EMIT(verified(false));
  QWebEngineView::closeEvent(event);
}

#include "facebookaccountverifyui.moc"
