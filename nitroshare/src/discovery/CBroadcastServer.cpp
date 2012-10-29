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

#include <QDateTime>

#include "discovery/CBroadcastServer.h"
#include "util/settings.h"

CBroadcastServer::CBroadcastServer()
{
    connect(&m_broadcaster, SIGNAL(Error(QString)), SIGNAL(Error(QString)));
    connect(&m_listener,    SIGNAL(Error(QString)), SIGNAL(Error(QString)));
    connect(&m_listener,    SIGNAL(Ping(QHostAddress,QVariantMap)), SLOT(OnPing(QHostAddress,QVariantMap)));

    connect(&m_timer, SIGNAL(timeout()), SLOT(OnTimer()));
}

void CBroadcastServer::Init()
{
    /* Stop the timeout timer. */
    m_timer.stop();

    /* Initialize the broadcaster and listener. */
    m_broadcaster.Init();
    m_listener.Init();

    /* The user has set an interval after which a machine is considered
       offline. We check for such machines at half of this interval. */
    m_timer.start(Settings::Get("Network/TimeoutInterval").toInt() * 500);

    OnTimer();
}

void CBroadcastServer::OnPing(QHostAddress address, QVariantMap map)
{
    QString id = map["id"].toString();

    /* Check to see if this machine is already in our map. If not
       then add it, checking to see if it has recently joined. */
    if(!m_machines.contains(id))
    {
        /* The CMachine entry will get filled in later. */
        m_machines.insert(id, CMachine());

        /* The new machine tells us what time it joined the network, so we
           can display a message if the user has enabled it. */
        if(map["uptime"].toLongLong() > m_broadcaster.GetUptime() && Settings::Notify("NewPC"))
            emit Information(tr("%1 has joined the network.").arg(QString(map["name"].toString())));
    }

    /* Update our entry for the machine. */
    m_machines[id].name      = map["name"].toByteArray();
    m_machines[id].address   = address;
    m_machines[id].port      = map["transmission_port"].toUInt();
}

void CBroadcastServer::OnTimer()
{
    /* Move through the map of machines, removing any that have expired. */
    QMutableMapIterator<QString, CMachine> i(m_machines);
    while(i.hasNext())
    {
        i.next();

        /* If the last ping from this machine + the user-specified timeout
           is greater than the current time, the machine is expired. */
        {
            if(Settings::Notify("PCQuit"))
                emit Information(tr("%1 has left the network.").arg(QString(i.value().name)));

            m_machines.remove(i.key());
        }
    }
}
