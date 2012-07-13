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

#include <QDebug>

#include "util/network.h"
#include "util/settings.h"

bool Network::FindIPv4Address(QNetworkInterface interface, QHostAddress * ip,
                              QHostAddress * broadcast, QHostAddress * netmask)
{
    /* Check each address to determine if it is a valid IPv4 address. */
    foreach(QNetworkAddressEntry entry, interface.addressEntries())
        if(entry.ip().toIPv4Address() &&
           entry.broadcast().toIPv4Address())
        {
            if(ip)        *ip        = entry.ip();
            if(broadcast) *broadcast = entry.broadcast();
            if(netmask)   *netmask   = entry.netmask();

            return true;
        }

    /* If we've come this far, there is no IPv4 addresses on the specified interface. */
    return false;
}

bool Network::IsInterfaceUsable(QNetworkInterface interface)
{
    /* We consider an interface usable if it meets the following criteria
       - we can broadcast on it
       - it's up & running
       - it isn't the loopback device (we may make this an option for testing sometime)
       - it contains at least one IPv4 address */
    return interface.flags() & (QNetworkInterface::CanBroadcast |
                                QNetworkInterface::IsUp |
                                QNetworkInterface::IsRunning) &&
           ~interface.flags() & QNetworkInterface::IsLoopBack &&
           FindIPv4Address(interface);
}

QList<QNetworkInterface> Network::FindUsableInterfaces()
{
    qDebug() << "Enumerating all network interfaces...";

    QList<QNetworkInterface> interfaces;
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        /* Determine if this interface is usable. */
        if(IsInterfaceUsable(interface))
        {
            qDebug() << " -" << interface.humanReadableName() << "is usable.";
            interfaces.append(interface);
        }
        else
            qDebug() << " -" << interface.humanReadableName() << "is NOT usable.";
    }

    return interfaces;
}

QString Network::FindUsableInterface()
{
    QList<QNetworkInterface> interfaces = FindUsableInterfaces();
    if(interfaces.size())
        return interfaces[0].name();

    return "";
}

QNetworkInterface Network::GetCurrentInterface()
{
    return QNetworkInterface::interfaceFromName(Settings::Get("Network/Interface").toString());
}
