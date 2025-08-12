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
#include <QApplication>
#include <QTimer>
#include <QDebug>

#include "facebookaccountverifyobserver.h"
#include "facebookaccountverifyui.h"

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTimer idle;
    int pending = 0;

    idle.setInterval(10000);
    idle.start();

    FacebookAccountVerifyObserver observer;

    QObject::connect(&observer, &FacebookAccountVerifyObserver::accountVerify,
    &app, [&](uintptr_t tag, const QString &url, const QString &title,
              const QString &message)
    {
      qDebug() << "New account verify request, tag"
               << Qt::hex << tag;
      idle.stop();
      pending++;

      FacebookAccountVerifyUI *ui = new FacebookAccountVerifyUI(url);

      QObject::connect(ui, &FacebookAccountVerifyUI::verified,
      [tag, &observer, &pending, &idle, ui](bool verified)
      {
        qDebug() << "Account verify completed, tag"
                 << Qt::hex << tag << verified;
        observer.verified(tag, verified);
        pending--;

        if (!pending)
          idle.start();

        ui->deleteLater();
      });
    }, Qt::QueuedConnection);

    observer.registerObserver();

    QObject::connect(&idle, &QTimer::timeout, []()
    {
      qDebug() << "Idle timeout expired, quit.";
      QApplication::quit();
    });

    return app.exec();
}
