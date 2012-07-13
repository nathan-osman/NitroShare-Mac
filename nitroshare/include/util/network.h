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

#ifndef NETWORK_H
#define NETWORK_H

#include <QHostAddress>
#include <QList>
#include <QNetworkInterface>

namespace Network {

    /* Attempts to find the first IPv4 address on the provided interface,
       optionally returning the IP, broadcast, and / or netmask if desired. */
    bool FindIPv4Address(QNetworkInterface, QHostAddress * = NULL, QHostAddress * = NULL, QHostAddress * = NULL);

    // Determines if the provided network interface is suitable for use
    bool IsInterfaceUsable(QNetworkInterface);

    // Retrieves a list of all usable network interfaces
    QList<QNetworkInterface> FindUsableInterfaces();

    // Selects the first network interface from the usable list
    QString FindUsableInterface();

    // Gets the current network interface
    QNetworkInterface GetCurrentInterface();
}

#endif // NETWORK_H
