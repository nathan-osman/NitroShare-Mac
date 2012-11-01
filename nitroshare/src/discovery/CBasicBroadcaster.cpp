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
#include <qjson/serializer.h>

#include "discovery/CBasicBroadcaster.h"
#include "util/definitions.h"
#include "util/network.h"
#include "util/settings.h"

CBasicBroadcaster::CBasicBroadcaster()
{
    connect(&m_timer, SIGNAL(timeout()), SLOT(OnTimer()));

    /* Store the current time (that this broadcaster was started with)
       to inform other machines of how long we have been running. */
	       m_uptime = (QDateTime().toTime_t() * 1000);

     }

void CBasicBroadcaster::Init()
{
    /* When this method is called without a network interface,
       we simply use GetCurrentInterface to either load the user's
       default interface or find one that is suitable for use. */
    Init(Network::GetCurrentInterface());
}

void CBasicBroadcaster::Init(QNetworkInterface interface)
{
    /* Stop the timer if it is currently running. */
    m_timer.stop();

    /* Attempt to find an IPv4 broadcast and netmask address on the interface. */
    if(!Network::FindIPv4Address(interface, NULL, &m_broadcast_address, &m_netmask_address))
    {
        emit Error(tr("Unable to find an IPv4 broadcast and netmask address for the current network interface."));
        return;
    }

    /* Grab the name of the interface. */
    m_name = interface.name();

    /* Start the timer and send out an initial ping. */
    m_timer.setInterval(Settings::Get("Network/BroadcastInterval").toInt());
    m_timer.start();

    OnTimer();
}

bool CBasicBroadcaster::HasAddress(QHostAddress address)
{
    return (m_broadcast_address.toIPv4Address() & m_netmask_address.toIPv4Address()) ==
           (address.toIPv4Address() & m_netmask_address.toIPv4Address());
}

void CBasicBroadcaster::OnTimer()
{
    /* Construct the JSON map containing some information about our instance. */
    QVariantMap info;
    info["version"]           = Definitions::ProtocolVersion;
    info["id"]                = Settings::GetID();
    info["name"]              = Settings::Get("General/MachineName");
    info["transmission_port"] = Settings::Get("Network/TransmissionPort");
    info["uptime"]            = m_uptime;

    /* Write the information to the broadcast port we discovered earlier. */
    writeDatagram(QJson::Serializer().serialize(QVariant(info)),
                  m_broadcast_address, Settings::Get("Network/BroadcastPort").toUInt());
}
