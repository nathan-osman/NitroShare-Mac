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

#ifndef CBROADCASTDISCOVERYWIDGET_H
#define CBROADCASTDISCOVERYWIDGET_H

#include <QList>
#include <QTimer>
#include <QWidget>

#include "discovery/CBasicBroadcaster.h"
#include "discovery/CBasicListener.h"

namespace Ui {
    class CBroadcastDiscoveryWidget;
}

class CBroadcastDiscoveryWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString interface READ GetInterface)
        
    public:

        CBroadcastDiscoveryWidget(QWidget *);
        virtual ~CBroadcastDiscoveryWidget();

        void StartBroadcasting();
        void StopBroadcasting();

        QString GetInterface() { return m_interface; }

    signals:

        void FoundInterface();

    private slots:

        void OnPing(QHostAddress);
        void OnTimer();

    private:

        Ui::CBroadcastDiscoveryWidget * ui;

        /* This list contains a bunch of broadcasters that continually
           send broadcast packets over all available network interfaces. */
        QList<CBasicBroadcaster *> m_broadcasters;

        /* ...and we use this listener to catch any incoming pings. */
        CBasicListener * m_listener;

        /* This is the interface we found. */
        QString m_interface;

        QTimer m_animation_timer;
        bool m_animation_frame;
};

#endif // CBROADCASTDISCOVERYWIDGET_H
