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

#include <QHostAddress>
#include <QNetworkInterface>

#include "file/CFileReceiver.h"
#include "file/CFileServer.h"
#include "util/network.h"
#include "util/settings.h"

CFileServer::CFileServer()
{
    //...
}

void CFileServer::Init()
{
    /* Stop listening if we already are listen on the new port. */
    close();

    /* Attempt to find the IPv4 address for the current network interface. */
    QHostAddress address;
    if(!Network::FindIPv4Address(Network::GetCurrentInterface(), &address))
    {
        emit Error(tr("Unable to find an IPv4 address on the current network interface."));
        return;
    }

    if(!listen(address, Settings::Get("Network/TransmissionPort").toUInt()))
        emit Error(tr("Unable to listen on TCP port %1.").arg(Settings::Get("Network/TransmissionPort").toUInt()));
}

void CFileServer::incomingConnection(int handle)
{
    CFileReceiver * receiver = new CFileReceiver(this);

    connect(receiver, SIGNAL(Error(QString)),       SIGNAL(Error(QString)));
    connect(receiver, SIGNAL(Information(QString)), SIGNAL(Information(QString)));

    receiver->Start(handle);
}
