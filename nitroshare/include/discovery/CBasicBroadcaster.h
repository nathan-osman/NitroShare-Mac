/* NitroShare - A simple network file sharing tool.
   Copyright (C) 2012 Nathan Osman

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef CBASICBROADCASTER_H
#define CBASICBROADCASTER_H

#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QUdpSocket>

class CBasicBroadcaster : public QUdpSocket
{
    Q_OBJECT

    public:

        CBasicBroadcaster();

        void Init();
        void Init(QNetworkInterface);

        bool HasAddress(QHostAddress);
        QString GetName() { return m_name; }
        qint64 GetUptime() { return m_uptime; }

    signals:

        void Error(QString);

    private slots:

        void OnTimer();

    private:

        QTimer m_timer;
        qint64 m_uptime;

        QString      m_name;
        QHostAddress m_broadcast_address;
        QHostAddress m_netmask_address;
};

#endif // CBASICBROADCASTER_H
