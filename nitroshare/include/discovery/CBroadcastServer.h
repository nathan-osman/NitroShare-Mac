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

#ifndef CBROADCASTSERVER_H
#define CBROADCASTSERVER_H

#include <QHostAddress>
#include <QTimer>
#include <QVariantMap>

#include "discovery/CBasicBroadcaster.h"
#include "discovery/CBasicListener.h"
#include "discovery/CMachine.h"

class CBroadcastServer : public QObject
{
    Q_OBJECT

    public:

        CBroadcastServer();

        void Init();

        /* Utility methods for dealing with the machines. */
        MachineMap GetMachineMap() { return m_machines; }
        bool AnyMachinesOnline() { return m_machines.size(); }
        bool MachineExists(QString id) { return m_machines.contains(id); }
        CMachine GetMachine(QString id) { return m_machines[id]; }

    signals:

        void Error(QString);
        void Information(QString);

    private slots:

        void OnPing(QHostAddress, QVariantMap);
        void OnTimer();

    private:

        CBasicBroadcaster m_broadcaster;
        CBasicListener    m_listener;

        MachineMap m_machines;

        QTimer m_timer;
};

#endif // CBROADCASTSERVER_H
