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
#include <QMessageBox>
#include <qjson/parser.h>

#include "discovery/CBasicListener.h"
#include "util/definitions.h"
#include "util/settings.h"

CBasicListener::CBasicListener()
{
    connect(this, SIGNAL(readyRead()), SLOT(OnReadyRead()));
}

void CBasicListener::Init()
{
    /* Stop listening on the socket if we currently are. */
    close();

    /* Make an attempt to bind to the user-specified broadcast port. */
    if(!bind(Settings::Get("Network/BroadcastPort").toUInt(), QUdpSocket::ShareAddress))
        emit Error(tr("Unable to listen for broadcast notifications."));
}

void CBasicListener::OnReadyRead()
{
    /* Determine the size of the incoming datagram and
       create room for it in a buffer. */
    QByteArray message;
    message.resize(pendingDatagramSize());

    /* Grab the message (and the address that sent it). */
    QHostAddress address;
    readDatagram(message.data(), message.size(), &address);

    /* Decode the JSON we received. */
    bool ok;
    QVariantMap map = QJson::Parser().parse(message, &ok).toMap();

    /* Make sure that there were no errors decoding the JSON and that
       we can read the results without any problems. */
    if(!ok || !map.contains("version") || map["version"].toInt() != Definitions::ProtocolVersion)
    {
        /* We only want to alert the user once. */
        static bool displayed_warning = false;

        if(!displayed_warning)
        {
            QMessageBox::warning(NULL, tr("Warning:"), tr("One of the machines on your network ('%1') is running a different version of NitroShare than this machine (%2). Please ensure both machines are running the latest version of NitroShare.")
                                                       .arg(address.toString()).arg(Definitions::Version));
            displayed_warning = true;
        }

        return;
    }

    /* Make sure none of the properties are ending. */
    if(!map.contains("id") || !map.contains("name") ||
       !map.contains("transmission_port") || !map.contains("uptime"))
    {
        qDebug() << "Ping received from" << address.toString() << "is invalid.";
        return;
    }

    /* Next, make sure that this is not one of _our_ packets. */
    if(map["id"] == Settings::GetID())
        return;

    /* Seems like a valid packet. */
    emit Ping(address, map);
}
