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
#include <QPixmap>

#include "util/network.h"
#include "widgets/CBroadcastDiscoveryWidget.h"
#include "ui_CBroadcastDiscoveryWidget.h"

CBroadcastDiscoveryWidget::CBroadcastDiscoveryWidget(QWidget * parent)
    : QWidget(parent), ui(new Ui::CBroadcastDiscoveryWidget), m_listener(NULL),
      m_animation_frame(false)
{
    ui->setupUi(this);

    connect(&m_animation_timer, SIGNAL(timeout()), SLOT(OnTimer()));
}

CBroadcastDiscoveryWidget::~CBroadcastDiscoveryWidget()
{
    /* Free everything. */
    StopBroadcasting();

    delete ui;
}

void CBroadcastDiscoveryWidget::StartBroadcasting()
{
    /* We may already be broadcasting, so stop. */
    StopBroadcasting();

    qDebug() << "Started broadcasting on all network interfaces.";
    ui->DiscoveryLabel->setText(tr("Searching for other machines..."));

    /* Listen for any incoming pings. */
    m_listener = new CBasicListener;
    connect(m_listener, SIGNAL(Ping(QHostAddress,QVariantMap)), SLOT(OnPing(QHostAddress)));

    m_listener->Init();

    /* For each of the usable network interfaces, create a broadcaster
       that immediately begins listening / broadcasting. */
    QList<QNetworkInterface> interfaces = Network::FindUsableInterfaces();
    foreach(QNetworkInterface interface, interfaces)
    {
        CBasicBroadcaster * broadcaster = new CBasicBroadcaster;
        broadcaster->Init(interface);

        m_broadcasters.append(broadcaster);
    }

    m_animation_timer.start(500);
}

void CBroadcastDiscoveryWidget::StopBroadcasting()
{
    qDebug() << "Stopped broadcasting on all network interfaces.";

    /* Free all of the broadcasters and the listener. */
    qDeleteAll(m_broadcasters);
    m_broadcasters.clear();

    if(m_listener)
    {
        delete m_listener;
        m_listener = NULL;
    }

    m_animation_timer.stop();
}

void CBroadcastDiscoveryWidget::OnPing(QHostAddress address)
{
    /* Loop through each of the network interfaces we are broadcasting
       on and check to see if this address is in its subnet. */
    foreach(CBasicBroadcaster * broadcaster, m_broadcasters)
        if(broadcaster->HasAddress(address))
        {
            ui->DiscoveryStatus->setPixmap(QPixmap(":/images/success.png"));
            ui->DiscoveryLabel->setText(tr("Network discovery complete!"));

            m_interface = broadcaster->GetName();

            emit FoundInterface();

            /* We could have called StopBroadcasting() here but then the
               other machines would stop receiving broadcast packets. We
               still need to stop the animation however. */
            m_animation_timer.stop();
        }
}

void CBroadcastDiscoveryWidget::OnTimer()
{
    ui->DiscoveryImage->setPixmap(QPixmap(QString(":/images/discovery%1.png").arg(m_animation_frame?1:2)));
    m_animation_frame = !m_animation_frame;
}
