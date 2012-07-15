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

#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QHostInfo>
#include <QVariantList>

#include "sharebox/CShareBox.h"
#include "util/defaults.h"
#include "util/network.h"

QVariantMap Defaults::Map;

QPoint Defaults::GetDefaultShareBoxPos()
{
    /* Get the dimensions of the primary screen. */
    QDesktopWidget * desktop = QApplication::desktop();
    QRect geometry = desktop->availableGeometry(desktop->primaryScreen());

    /* Now calculate the position. */
    QPoint pos(geometry.right() - CShareBox::DefaultWidth - 30,
               geometry.bottom() - CShareBox::DefaultHeight - 30);

    /* Round to the nearest 10 pixels. */
    pos /= 10;
    pos *= 10;

    return pos;
}

QVariant GenerateDefaultShareBoxInstance()
{
    /* Generate the map/list that we will return. */
    QVariantMap map;
    map["pos"] = Defaults::GetDefaultShareBoxPos();

    QVariantList list;
    list << map;

    return QVariant(list);
}

void Defaults::Init()
{
    Map["General/MachineName"]            = QHostInfo::localHostName();
    Map["General/ReceivedFilesDirectory"] = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    Map["General/CompressFiles"]          = false;
    Map["General/CalculateChecksum"]      = false;

    Map["Appearance/Icon"] = IconDark;

    Map["Notifications/NewPC"]         = false;
    Map["Notifications/PCQuit"]        = false;
    Map["Notifications/FilesReceived"] = true;
    Map["Notifications/Error"]         = true;

    Map["Security/IncomingPolicy"]  = AcceptPrompt;
    Map["Security/IncomingPattern"] = "";

    Map["Network/Interface"]         = Network::FindUsableInterface();
    Map["Network/BroadcastPort"]     = 41720;
    Map["Network/BroadcastInterval"] = 4000;
    Map["Network/TimeoutInterval"]   = 45;
    Map["Network/TransmissionPort"]  = 41721;

    Map["RPC/Port"] = 41722;

    Map["ShareBox/Instances"] = GenerateDefaultShareBoxInstance();
}
