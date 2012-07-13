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

#ifndef CMACHINE_H
#define CMACHINE_H

#include <QHostAddress>
#include <QMap>

class CMachine
{
    public:

        QByteArray name;      // the hostname of the machine
        qint64 last_ping;     // the time of the last ping for the machine
        QHostAddress address; // the address the last ping was received from
        quint16 port;         // the port the machine is listening for file transfers on
};

/* The machine map maps the unique ID of each machine to
   its appropriate CMachine instance. */
typedef QMap<QString, CMachine> MachineMap;

#endif // CMACHINE_H
